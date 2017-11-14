#include "pinchers.h"

using namespace std;

#define PISTON_LOC 3

Pinchers::Input::Input():enabled(false){}
Pinchers::Input::Input(bool a):enabled(a){}

Pinchers::Estimator::Estimator():last(Status_detail::OPEN){
}

std::set<Pinchers::Output> examples(Pinchers::Output*){
	return {
		#define X(NAME) Pinchers::Output::NAME,
		PINCHERS_OUTPUTS(X)
		#undef X
	};
}

ostream& operator<<(ostream& o,Pinchers::Output a){
	#define X(A) if(a==Pinchers::Output::A) return o<<""#A;
	PINCHERS_OUTPUTS(X)
	#undef X
	assert(0);
}

std::set<Pinchers::Goal> examples(Pinchers::Goal*){
	return {
		#define X(NAME) Pinchers::Goal::NAME,
		PINCHERS_GOALS(X)
		#undef X
	};
}

std::set<Pinchers::Input> examples(Pinchers::Input*){
	return {
		{true},
		{false}
	};
}

std::set<Pinchers::Status_detail> examples(Pinchers::Status_detail*){
	return {Pinchers::Status_detail::OPEN,Pinchers::Status_detail::OPENING,Pinchers::Status_detail::CLOSING,Pinchers::Status_detail::CLOSED};
}

std::ostream& operator<<(std::ostream& o,Pinchers::Goal g){
	#define X(name) if(g==Pinchers::Goal::name) return o<<""#name;
	PINCHERS_GOALS(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Pinchers::Input a){
	return o<<"Input(enabled:"<<a.enabled<<")";
}

std::ostream& operator<<(std::ostream& o,Pinchers::Status_detail a){
	#define X(STATUS) if(a==Pinchers::Status_detail::STATUS) return o<<""#STATUS;
	X(OPEN)
	X(OPENING)
	X(CLOSING)
	X(CLOSED)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Pinchers const&){
	return o<<"Pinchers()";
}

bool operator<(Pinchers::Input a,Pinchers::Input b){ 
	return !a.enabled && b.enabled;
}
bool operator==(Pinchers::Input a,Pinchers::Input b){
	return a.enabled == b.enabled;
}
bool operator!=(Pinchers::Input a, Pinchers::Input b){ return !(a==b); }

bool operator==(Pinchers::Estimator,Pinchers::Estimator){ return 1; }
bool operator!=(Pinchers::Estimator a, Pinchers::Estimator b){ return !(a==b); }
ostream& operator<<(ostream& o, Pinchers::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" state_timer:"<<a.state_timer<<")";
}

bool operator==(Pinchers,Pinchers){ return 1; }
bool operator!=(Pinchers a, Pinchers b){ return !(a==b); }

Pinchers::Input Pinchers::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.robot_mode.enabled};
}

Robot_inputs Pinchers::Input_reader::operator()(Robot_inputs r, Pinchers::Input in) const{
	r.robot_mode.enabled = in.enabled;
	return r;
}

Robot_outputs Pinchers::Output_applicator::operator()(Robot_outputs r, Pinchers::Output o)const{
	r.solenoid[PISTON_LOC] = o == Pinchers::Output::CLOSE;
	return r;
}

Pinchers::Output Pinchers::Output_applicator::operator()(Robot_outputs const& r)const{
	return r.solenoid[PISTON_LOC] ? Output::CLOSE : Output::OPEN;
}

void Pinchers::Estimator::update(Time time,Pinchers::Input input,Pinchers::Output output){
	switch(output){
		case Pinchers::Output::CLOSE:
			if(last == Status::CLOSING){
				state_timer.update(time,input.enabled);
			} else if(last != Status::CLOSED){ 
				const Time DOWN_TIME = 1.0;//seconds. assumed
				last = Status::CLOSING;
				state_timer.set(DOWN_TIME);
			}
			if(state_timer.done() || last == Status::CLOSED) {
				last = Status::CLOSED;
			}
			break;
		case Pinchers::Output::OPEN:
			if(last == Status::OPENING){
				state_timer.update(time,input.enabled);
			} else if(last != Status::OPEN){ 
				const Time UP_TIME = 2.2;//seconds. assumed
				last = Status::OPENING;
				state_timer.set(UP_TIME);
			}
			if(state_timer.done() || last == Status::OPEN) { 
				last = Status::OPEN;
			}
			break;
		default:
			assert(0);
	}
}

Pinchers::Status Pinchers::Estimator::get()const{
	return last;
}

Pinchers::Output control(Pinchers::Status status,Pinchers::Goal goal){
	switch(goal){
		case Pinchers::Goal::CLOSE:
			return Pinchers::Output::CLOSE;
		case Pinchers::Goal::OPEN:
			return Pinchers::Output::OPEN;
		case Pinchers::Goal::X:
			switch(status){
				case Pinchers::Status::OPEN:
				case Pinchers::Status::OPENING:
					return Pinchers::Output::OPEN;
				case Pinchers::Status::CLOSING:
				case Pinchers::Status::CLOSED:
					return Pinchers::Output::CLOSE;
				default:
					assert(0);
			}
		default:
			assert(0);
	}
}

Pinchers::Status status(Pinchers::Status s){
	return s;
}

bool ready(Pinchers::Status status,Pinchers::Goal goal){
	switch(goal){
		case Pinchers::Goal::OPEN:
			return status == Pinchers::Status::OPEN;
		case Pinchers::Goal::CLOSE:
			return status == Pinchers::Status::CLOSED;
		case Pinchers::Goal::X:
			return 1;
		default:
			assert(0);
	}
	return false;
}

#ifdef PINCHERS_TEST
#include "formal.h"
int main(){
	{
		Pinchers a;
		tester(a);
	}
	{
		Pinchers a;
		Pinchers::Goal goal = Pinchers::Goal::CLOSE;

		const bool ENABLED = true;	
		for(Time t: range(100)){
			Pinchers::Status_detail status = a.estimator.get();
			Pinchers::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Pinchers::Input{ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}
		}

		goal = Pinchers::Goal::OPEN;
		
		for(Time t: range(100)){
			Pinchers::Status_detail status = a.estimator.get();
			Pinchers::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Pinchers::Input{ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}

		}
	}
}
#endif
