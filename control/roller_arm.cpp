#include "roller_arm.h"

using namespace std;

#define PISTON_LOC 3

Roller_arm::Input::Input():enabled(false){}
Roller_arm::Input::Input(bool a):enabled(a){}

Roller_arm::Estimator::Estimator():last(Status_detail::STOW){
}

std::set<Roller_arm::Output> examples(Roller_arm::Output*){
	return {
		#define X(NAME) Roller_arm::Output::NAME,
		ROLLER_ARM_OUTPUTS(X)
		#undef X
	};
}

ostream& operator<<(ostream& o,Roller_arm::Output a){
	#define X(A) if(a==Roller_arm::Output::A) return o<<""#A;
	ROLLER_ARM_OUTPUTS(X)
	#undef X
	assert(0);
}

std::set<Roller_arm::Goal> examples(Roller_arm::Goal*){
	return {
		#define X(NAME) Roller_arm::Goal::NAME,
		ROLLER_ARM_GOALS(X)
		#undef X
	};
}

std::set<Roller_arm::Input> examples(Roller_arm::Input*){
	return {
		{true},
		{false}
	};
}

std::set<Roller_arm::Status_detail> examples(Roller_arm::Status_detail*){
	return {Roller_arm::Status_detail::STOW,Roller_arm::Status_detail::STOWING,Roller_arm::Status_detail::LOWERING,Roller_arm::Status_detail::LOW};
}

std::ostream& operator<<(std::ostream& o,Roller_arm::Goal g){
	#define X(name) if(g==Roller_arm::Goal::name) return o<<""#name;
	ROLLER_ARM_GOALS(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Roller_arm::Input a){
	return o<<"Input(enabled:"<<a.enabled<<")";
}

std::ostream& operator<<(std::ostream& o,Roller_arm::Status_detail a){
	#define X(STATUS) if(a==Roller_arm::Status_detail::STATUS) return o<<""#STATUS;
	X(STOW)
	X(STOWING)
	X(LOWERING)
	X(LOW)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Roller_arm const&){
	return o<<"Roller_arm()";
}

bool operator<(Roller_arm::Input a,Roller_arm::Input b){ 
	return !a.enabled && b.enabled;
}
bool operator==(Roller_arm::Input a,Roller_arm::Input b){
	return a.enabled == b.enabled;
}
bool operator!=(Roller_arm::Input a, Roller_arm::Input b){ return !(a==b); }

bool operator==(Roller_arm::Estimator,Roller_arm::Estimator){ return 1; }
bool operator!=(Roller_arm::Estimator a, Roller_arm::Estimator b){ return !(a==b); }
ostream& operator<<(ostream& o, Roller_arm::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" state_timer:"<<a.state_timer<<")";
}

bool operator==(Roller_arm,Roller_arm){ return 1; }
bool operator!=(Roller_arm a, Roller_arm b){ return !(a==b); }

Roller_arm::Input Roller_arm::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.robot_mode.enabled};
}

Robot_inputs Roller_arm::Input_reader::operator()(Robot_inputs r, Roller_arm::Input in) const{
	r.robot_mode.enabled = in.enabled;
	return r;
}

Robot_outputs Roller_arm::Output_applicator::operator()(Robot_outputs r, Roller_arm::Output o)const{
	r.solenoid[PISTON_LOC] = o == Roller_arm::Output::LOW;
	return r;
}

Roller_arm::Output Roller_arm::Output_applicator::operator()(Robot_outputs const& r)const{
	return r.solenoid[PISTON_LOC] ? Output::LOW : Output::STOW;
}

void Roller_arm::Estimator::update(Time time,Roller_arm::Input input,Roller_arm::Output output){
	switch(output){
		case Roller_arm::Output::LOW:
			if(last == Status::LOWERING){
				state_timer.update(time,input.enabled);
			} else if(last != Status::LOW){ 
				const Time LOW_TIME = .6;//seconds. assumed
				last = Status::LOWERING;
				state_timer.set(LOW_TIME);
			}
			if(state_timer.done() || last == Status::LOW) {
				last = Status::LOW;
			}
			break;
		case Roller_arm::Output::STOW:
			if(last == Status::STOWING){
				state_timer.update(time,input.enabled);
			} else if(last != Status::STOW){ 
				const Time STOW_TIME = 1;//seconds. assumed
				last = Status::STOWING;
				state_timer.set(STOW_TIME);
			}
			if(state_timer.done() || last == Status::STOW) { 
				last = Status::STOW;
			}
			break;
		default:
			assert(0);
	}
}

Roller_arm::Status Roller_arm::Estimator::get()const{
	return last;
}

Roller_arm::Output control(Roller_arm::Status status,Roller_arm::Goal goal){
	switch(goal){
		case Roller_arm::Goal::STOW:
			return Roller_arm::Output::STOW;
		case Roller_arm::Goal::LOW:
			return Roller_arm::Output::LOW;
		case Roller_arm::Goal::X:
			switch(status){
				case Roller_arm::Status::STOW:
				case Roller_arm::Status::STOWING:
					return Roller_arm::Output::STOW;
				case Roller_arm::Status::LOWERING:
				case Roller_arm::Status::LOW:
					return Roller_arm::Output::LOW;
				default:
					assert(0);
			}
		default:
			assert(0);
	}
}

Roller_arm::Status status(Roller_arm::Status s){
	return s;
}

bool ready(Roller_arm::Status status,Roller_arm::Goal goal){
	switch(goal){
		case Roller_arm::Goal::STOW:
			return status == Roller_arm::Status::STOW;
		case Roller_arm::Goal::LOW:
			return status == Roller_arm::Status::LOW;
		case Roller_arm::Goal::X:
			return 1;
		default:
			assert(0);
	}
	return false;
}

#ifdef ROLLER_ARM_TEST
#include "formal.h"
int main(){
	{
		Roller_arm a;
		tester(a);
	}
	{
		Roller_arm a;
		Roller_arm::Goal goal = Roller_arm::Goal::LOW;

		const bool ENABLED = true;	
		for(Time t: range(100)){
			Roller_arm::Status_detail status = a.estimator.get();
			Roller_arm::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Roller_arm::Input{ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}
		}

		goal = Roller_arm::Goal::STOW;
		
		for(Time t: range(100)){
			Roller_arm::Status_detail status = a.estimator.get();
			Roller_arm::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Roller_arm::Input{ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}

		}
	}
}
#endif
