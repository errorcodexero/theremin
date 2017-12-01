#include "grabber_arm.h"

using namespace std;

#define PISTON_LOC 2
#define GRABBER_ARM_LIMIT 5

Grabber_arm::Input::Input():enabled(false),limit_switch(false){}
Grabber_arm::Input::Input(bool a,bool b):enabled(a),limit_switch(b){}

Grabber_arm::Estimator::Estimator():last(Status_detail::UP){
}

std::set<Grabber_arm::Output> examples(Grabber_arm::Output*){
	return {
		#define X(NAME) Grabber_arm::Output::NAME,
		GRABBER_ARM_OUTPUTS(X)
		#undef X
	};
}

ostream& operator<<(ostream& o,Grabber_arm::Output a){
	#define X(A) if(a==Grabber_arm::Output::A) return o<<""#A;
	GRABBER_ARM_OUTPUTS(X)
	#undef X
	assert(0);
}

std::set<Grabber_arm::Goal> examples(Grabber_arm::Goal*){
	return {
		#define X(NAME) Grabber_arm::Goal::NAME,
		GRABBER_ARM_GOALS(X)
		#undef X
	};
}

std::set<Grabber_arm::Input> examples(Grabber_arm::Input*){
	return {
		{true,true},
		{true,false},
		{false,true},
		{false,false}
	};
}

std::set<Grabber_arm::Status_detail> examples(Grabber_arm::Status_detail*){
	return {Grabber_arm::Status_detail::DOWN,Grabber_arm::Status_detail::GOING_DOWN,Grabber_arm::Status_detail::GOING_UP,Grabber_arm::Status_detail::UP};
}

std::ostream& operator<<(std::ostream& o,Grabber_arm::Goal g){
	#define X(name) if(g==Grabber_arm::Goal::name) return o<<""#name;
	GRABBER_ARM_GOALS(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Grabber_arm::Input a){
	o<<"Input(";
	o<<"enabled:"<<a.enabled;
	o<<" limit_switch:"<<a.limit_switch;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Grabber_arm::Status_detail a){
	#define X(STATUS) if(a==Grabber_arm::Status_detail::STATUS) return o<<""#STATUS;
	X(UP)
	X(GOING_UP)
	X(GOING_DOWN)
	X(DOWN)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Grabber_arm const&){
	return o<<"Grabber_arm()";
}

bool operator<(Grabber_arm::Input a,Grabber_arm::Input b){ 
	if(b.enabled && !a.enabled) return true;
	if(a.enabled && !b.enabled) return false;
	if(b.limit_switch && !a.limit_switch) return true;
	if(a.limit_switch && !b.limit_switch) return false;
	return false;
}
bool operator==(Grabber_arm::Input a,Grabber_arm::Input b){
	return a.enabled == b.enabled && a.limit_switch == b.limit_switch;
}
bool operator!=(Grabber_arm::Input a, Grabber_arm::Input b){ return !(a==b); }

bool operator==(Grabber_arm::Estimator,Grabber_arm::Estimator){ return 1; }
bool operator!=(Grabber_arm::Estimator a, Grabber_arm::Estimator b){ return !(a==b); }
ostream& operator<<(ostream& o, Grabber_arm::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" state_timer:"<<a.state_timer<<")";
}

bool operator==(Grabber_arm,Grabber_arm){ return 1; }
bool operator!=(Grabber_arm a, Grabber_arm b){ return !(a==b); }

Grabber_arm::Input Grabber_arm::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.robot_mode.enabled,r.digital_io.in[GRABBER_ARM_LIMIT]==Digital_in::_1};
}

Robot_inputs Grabber_arm::Input_reader::operator()(Robot_inputs r, Grabber_arm::Input in) const{
	r.robot_mode.enabled = in.enabled;
	r.digital_io.in[GRABBER_ARM_LIMIT] = in.limit_switch ? Digital_in::_1 : Digital_in::_0;
	return r;
}

Robot_outputs Grabber_arm::Output_applicator::operator()(Robot_outputs r, Grabber_arm::Output o)const{
	r.solenoid[PISTON_LOC] = o == Grabber_arm::Output::UP;
	return r;
}

Grabber_arm::Output Grabber_arm::Output_applicator::operator()(Robot_outputs const& r)const{
	return r.solenoid[PISTON_LOC] ? Output::UP : Output::DOWN;
}

void Grabber_arm::Estimator::update(Time time,Grabber_arm::Input input,Grabber_arm::Output output){
	switch(output){
		case Grabber_arm::Output::DOWN:
			if(last != Status::DOWN) last = Status::GOING_DOWN;
			if(input.limit_switch || last == Status::DOWN) last = Status::DOWN;
			break;
		case Grabber_arm::Output::UP:
			if(last == Status::GOING_UP){
				state_timer.update(time,input.enabled);
			} else if(last != Status::UP){ 
				const Time UP_TIME = 2.2;//seconds. assumed
				last = Status::GOING_UP;
				state_timer.set(UP_TIME);
			}
			if(state_timer.done() || last == Status::UP) { 
				last = Status::UP;
			}
			break;
		default:
			assert(0);
	}
}

Grabber_arm::Status Grabber_arm::Estimator::get()const{
	return last;
}

Grabber_arm::Output control(Grabber_arm::Status status,Grabber_arm::Goal goal){
	switch(goal){
		case Grabber_arm::Goal::UP:
			return Grabber_arm::Output::UP;
		case Grabber_arm::Goal::DOWN:
			return Grabber_arm::Output::DOWN;
		case Grabber_arm::Goal::X:
			switch(status){
				case Grabber_arm::Status::UP:
				case Grabber_arm::Status::GOING_UP:
					return Grabber_arm::Output::UP;
				case Grabber_arm::Status::GOING_DOWN:
				case Grabber_arm::Status::DOWN:
					return Grabber_arm::Output::DOWN;
				default:
					assert(0);
			}
		default:
			assert(0);
	}
}

Grabber_arm::Status status(Grabber_arm::Status s){
	return s;
}

bool ready(Grabber_arm::Status status,Grabber_arm::Goal goal){
	switch(goal){
		case Grabber_arm::Goal::UP:
			return status == Grabber_arm::Status::UP;
		case Grabber_arm::Goal::DOWN:
			return status == Grabber_arm::Status::DOWN;
		case Grabber_arm::Goal::X:
			return 1;
		default:
			assert(0);
	}
	return false;
}

#ifdef GRABBER_ARM_TEST
#include "formal.h"
int main(){
	{
		Grabber_arm a;
		tester(a);
	}
	{
		Grabber_arm a;
		Grabber_arm::Goal goal = Grabber_arm::Goal::UP;

		const bool ENABLED = true;	
		for(Time t: range(100)){
			Grabber_arm::Status_detail status = a.estimator.get();
			Grabber_arm::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Grabber_arm::Input{ENABLED,t<.2},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}
		}

		goal = Grabber_arm::Goal::DOWN;
		
		for(Time t: range(100)){
			Grabber_arm::Status_detail status = a.estimator.get();
			Grabber_arm::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Grabber_arm::Input{ENABLED,t>2},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}

		}
	}
}
#endif
