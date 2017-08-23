#include "gear_lifter.h"

using namespace std;

#define PISTON_LOC 2
#define GEAR_LIFTER_LIMIT 5

Gear_lifter::Input::Input():enabled(false){}
Gear_lifter::Input::Input(bool e,bool l):enabled(e),limit_switch(l){}

Gear_lifter::Estimator::Estimator(){
	last = Status_detail::DOWN;
}

std::set<Gear_lifter::Goal> examples(Gear_lifter::Goal*){
	return {
		#define X(NAME) Gear_lifter::Goal::NAME,
		GEAR_LIFTER_GOALS(X)
		#undef X
	};
}

std::set<Gear_lifter::Output> examples(Gear_lifter::Output*){
	return {
		#define X(NAME) Gear_lifter::Output::NAME,
		GEAR_LIFTER_OUTPUTS(X)
		#undef X
	};
}

ostream& operator<<(ostream& o,Gear_lifter::Output a){
	#define X(A) if(a==Gear_lifter::Output::A) return o<<""#A;
	GEAR_LIFTER_OUTPUTS(X)
	#undef X
	assert(0);
}

std::set<Gear_lifter::Input> examples(Gear_lifter::Input*){
	return {
		{false, false},
		{false, true},
		{true, false},
		{true, true}
	};
}

std::set<Gear_lifter::Status_detail> examples(Gear_lifter::Status_detail*){
	return {Gear_lifter::Status_detail::DOWN,Gear_lifter::Status_detail::GOING_DOWN,Gear_lifter::Status_detail::GOING_UP,Gear_lifter::Status_detail::UP};
}

std::ostream& operator<<(std::ostream& o,Gear_lifter::Goal g){
	#define X(name) if(g==Gear_lifter::Goal::name) return o<<""#name;
	GEAR_LIFTER_GOALS(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Gear_lifter::Input a){
	return o<<"Input(enabled:"<<a.enabled<<")";
}

std::ostream& operator<<(std::ostream& o,Gear_lifter::Status_detail a){
	#define X(STATUS) if(a==Gear_lifter::Status_detail::STATUS) return o<<""#STATUS;
	X(DOWN)
	X(GOING_DOWN)
	X(GOING_UP)
	X(UP)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Gear_lifter const&){
	return o<<"Gear_lifter()";
}

bool operator<(Gear_lifter::Input a,Gear_lifter::Input b){ 
	return !a.enabled && b.enabled;
}
bool operator==(Gear_lifter::Input a,Gear_lifter::Input b){
	return a.enabled == b.enabled;
}
bool operator!=(Gear_lifter::Input a, Gear_lifter::Input b){ return !(a==b); }

bool operator==(Gear_lifter::Estimator,Gear_lifter::Estimator){ return 1; }
bool operator!=(Gear_lifter::Estimator a, Gear_lifter::Estimator b){ return !(a==b); }
ostream& operator<<(ostream& o, Gear_lifter::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" state_timer:"<<a.state_timer<<")";
}

bool operator==(Gear_lifter,Gear_lifter){ return 1; }
bool operator!=(Gear_lifter a, Gear_lifter b){ return !(a==b); }

Gear_lifter::Input Gear_lifter::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.robot_mode.enabled, r.digital_io.in[GEAR_LIFTER_LIMIT]==Digital_in::_0};
}

Robot_inputs Gear_lifter::Input_reader::operator()(Robot_inputs r, Gear_lifter::Input in) const{
	r.robot_mode.enabled = in.enabled;
	r.digital_io.in[GEAR_LIFTER_LIMIT] = in.limit_switch?Digital_in::_0:Digital_in::_1;
	return r;
}

Robot_outputs Gear_lifter::Output_applicator::operator()(Robot_outputs r, Gear_lifter::Output o)const{
	r.solenoid[PISTON_LOC] = o == Gear_lifter::Output::UP;
	return r;
}

Gear_lifter::Output Gear_lifter::Output_applicator::operator()(Robot_outputs const& r)const{
	return r.solenoid[PISTON_LOC] ? Output::UP : Output::DOWN;
}

void Gear_lifter::Estimator::update(Time time,Gear_lifter::Input input,Gear_lifter::Output output){
	switch(output){
		case Gear_lifter::Output::UP:
			if(last == Status::GOING_UP){
				state_timer.update(time,input.enabled);
			} else if(last != Status::UP){ 
				const Time UP_TIME = .6;//seconds. tested
				last = Status::GOING_UP;
				state_timer.set(UP_TIME);
			}
			if(state_timer.done() || last == Status::UP) {
				last = Status::UP;
			}
			break;
		case Gear_lifter::Output::DOWN:
			/*
			if(last == Status::GOING_DOWN){
				state_timer.update(time,input.enabled);
			} else if(last != Status::DOWN){ 
				const Time DOWN_TIME = .6;//seconds. tested
				last = Status::GOING_DOWN;
				state_timer.set(DOWN_TIME);
			}
			if(state_timer.done() || last == Status::DOWN) { 
				last = Status::DOWN;
			}*/
			if(last != Status::DOWN) last = Status::GOING_DOWN;
			if(input.limit_switch || last == Status::DOWN) last = Status::DOWN;
			break;
		default:
			assert(0);
	}
}

Gear_lifter::Status Gear_lifter::Estimator::get()const{
	return last;
}

Gear_lifter::Output control(Gear_lifter::Status status,Gear_lifter::Goal goal){
	switch(goal){
		case Gear_lifter::Goal::UP:
			return Gear_lifter::Output::UP;
		case Gear_lifter::Goal::DOWN:
			return Gear_lifter::Output::DOWN;
		case Gear_lifter::Goal::X:
			switch(status){
				case Gear_lifter::Status::DOWN:
				case Gear_lifter::Status::GOING_DOWN:
					return Gear_lifter::Output::DOWN;
				case Gear_lifter::Status::GOING_UP:
				case Gear_lifter::Status::UP:
					return Gear_lifter::Output::UP;
				default:
					assert(0);
			}
		default:
			assert(0);
	}
}

Gear_lifter::Status status(Gear_lifter::Status s){
	return s;
}

bool ready(Gear_lifter::Status status,Gear_lifter::Goal goal){
	switch(goal){
		case Gear_lifter::Goal::DOWN:
			return status == Gear_lifter::Status::DOWN;
		case Gear_lifter::Goal::UP:
			return status == Gear_lifter::Status::UP;
		case Gear_lifter::Goal::X:
			return 1;
		default:
			assert(0);
	}
	return false;
}

#ifdef GEAR_LIFTER_TEST
#include "formal.h"
int main(){
	{
		Gear_lifter a;
		tester(a);
	}
	{
		Gear_lifter a;
		Gear_lifter::Goal goal = Gear_lifter::Goal::UP;

		const bool ENABLED = true;	
		for(Time t: range(100)){
			Gear_lifter::Status_detail status = a.estimator.get();
			Gear_lifter::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Gear_lifter::Input{ENABLED,false},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}
		}

		goal = Gear_lifter::Goal::DOWN;
		
		for(Time t: range(100)){
			Gear_lifter::Status_detail status = a.estimator.get();
			Gear_lifter::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Gear_lifter::Input{ENABLED,t>2},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}

		}
	}
}
#endif
