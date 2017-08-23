#include "gear_grabber.h"

using namespace std;

#define GEAR_LIGHT_OUT 0 //-1
#define GEAR_SENSOR_LOC 4
#define PISTON_LOC 1

Gear_grabber::Input::Input():has_gear(false),enabled(false){}
Gear_grabber::Input::Input(bool a,bool b):has_gear(a),enabled(b){}

Gear_grabber::Output::Output():piston(Gear_grabber::Output::Piston::OPEN),gear_light(false){}
Gear_grabber::Output::Output(Gear_grabber::Output::Piston a,bool b):piston(a),gear_light(b){}

Gear_grabber::Estimator::Estimator():last(){
	const Time OPEN_TIME = .2, CLOSE_TIME = .2;
	open_timer.set(OPEN_TIME);
	close_timer.set(CLOSE_TIME);
}

ostream& operator<<(ostream& o,Gear_grabber::Output::Piston a){
	#define X(A) if(a==Gear_grabber::Output::Piston::A) return o<<""#A;
	GEAR_GRABBER_PISTON_OUTPUTS(X)
	#undef X
	assert(0);
}

std::set<Gear_grabber::Output::Piston> examples(Gear_grabber::Output::Piston*){
	return {
		#define X(A) Gear_grabber::Output::Piston::A,
		GEAR_GRABBER_PISTON_OUTPUTS(X)
		#undef X
	};
}

bool operator==(const Gear_grabber::Input a,const Gear_grabber::Input b){
	if(a.has_gear != b.has_gear) return false;
	return a.enabled == b.enabled;
}

bool operator!=(const Gear_grabber::Input a,const Gear_grabber::Input b){
	return !(a==b);
}

bool operator<(const Gear_grabber::Input a,const Gear_grabber::Input b){
	if(a.has_gear && !b.has_gear) return false;
	return !a.enabled && b.enabled;
}

bool operator==(const Gear_grabber::Estimator a,const Gear_grabber::Estimator b){
	if(a.last != b.last) return false;
	return a.open_timer == b.open_timer;
}

bool operator!=(const Gear_grabber::Estimator a,const Gear_grabber::Estimator b){
	return !(a==b);
}

bool operator==(Gear_grabber::Output const& a,Gear_grabber::Output const& b){
	return a.piston == b.piston && a.gear_light == b.gear_light;
}

bool operator!=(Gear_grabber::Output const& a,Gear_grabber::Output const& b){
	return !(a==b);
}

ostream& operator<<(ostream& o, const Gear_grabber::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" open_timer:"<<a.open_timer<<")";
}

ostream& operator<<(ostream& o,const Gear_grabber::Input a){
	return o<<"Input(has_gear:"<<a.has_gear<<" enabled:"<<a.enabled<<")";
}

ostream& operator<<(ostream& o,const Gear_grabber::Goal a){
	#define X(GOAL) if(a==Gear_grabber::Goal::GOAL) return o<<""#GOAL;
	X(OPEN) X(CLOSE)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,const Gear_grabber::Status_detail a){
	#define X(STATUS) if(a.state==Gear_grabber::Status_detail::State::STATUS) return o<<"Status_detail(state:"#STATUS<<" has_gear:"<<a.has_gear<<")";
	X(OPEN)
	X(OPENING)
	X(CLOSING)
	X(CLOSED)
	#undef X
	assert(0);
}

bool operator==(Gear_grabber::Status_detail const& a,Gear_grabber::Status_detail const& b){
	return a.state == b.state && a.has_gear == b.has_gear;
}

bool operator!=(Gear_grabber::Status_detail const& a,Gear_grabber::Status_detail const& b){
	return !(a==b);
}

bool operator<(Gear_grabber::Output const& a,Gear_grabber::Output const& b){
	if(a.piston < b.piston) return true;
	if(a.piston > b.piston) return false;
	return !a.gear_light && b.gear_light;
}

bool operator<(Gear_grabber::Status_detail const& a,Gear_grabber::Status_detail const& b){
	if(a.state < b.state) return true;
	if(a.state > b.state) return false;
	return !a.has_gear && b.has_gear;
}

ostream& operator<<(ostream& o,Gear_grabber::Output const& a){
	return o<<"("<<a.piston<<" "<<a.gear_light<<")";
}

Gear_grabber::Status_detail::Status_detail():state(Gear_grabber::Status_detail::State::OPEN),has_gear(false){}
Gear_grabber::Status_detail::Status_detail(Gear_grabber::Status_detail::State s,bool g):state(s),has_gear(g){}

set<Gear_grabber::Input> examples(Gear_grabber::Input*){
	return {
		{false,false},
		{true,false},
		{false,true},
		{true,true}
	};
}

set<Gear_grabber::Goal> examples(Gear_grabber::Goal*){
	return {Gear_grabber::Goal::OPEN,Gear_grabber::Goal::CLOSE};
}

set<Gear_grabber::Output> examples(Gear_grabber::Output*){
	set<Gear_grabber::Output> outs;
	for(Gear_grabber::Output::Piston a: examples((Gear_grabber::Output::Piston*)nullptr)){
		outs.insert({a,true});
		outs.insert({a,false});
	}
	return outs;
}

set<Gear_grabber::Status_detail> examples(Gear_grabber::Status_detail*){
	set<Gear_grabber::Status_detail::State> states = {
		Gear_grabber::Status_detail::State::OPEN,
		Gear_grabber::Status_detail::State::OPENING,
		Gear_grabber::Status_detail::State::CLOSING,
		Gear_grabber::Status_detail::State::CLOSED
	};
	set<Gear_grabber::Status_detail> statuses;
	for(Gear_grabber::Status_detail::State a: states){
		statuses.insert({a,true});
		statuses.insert({a,false});
	}
	return statuses;
}

ostream& operator<<(ostream& o,Gear_grabber a){
	return o<<"Gear_grabber("<<a.estimator.get()<<")";
}

Gear_grabber::Status_detail Gear_grabber::Estimator::get()const{
	return last;
}

void Gear_grabber::Estimator::update(Time time,Input input,Output output){
	switch(output.piston){
		case Gear_grabber::Output::Piston::OPEN:
			if(last.state == Gear_grabber::Status_detail::State::OPENING){
				open_timer.update(time,input.enabled);
			} else if(last.state != Gear_grabber::Status_detail::State::OPEN){
				const Time OPEN_TIME = .15;//seconds .tested 
				open_timer.set(OPEN_TIME);
				last.state = Status_detail::State::OPENING;
			}
			if(open_timer.done()){
				 last.state = Gear_grabber::Status_detail::State::OPEN;
			}
			break;
		case Gear_grabber::Output::Piston::CLOSE:
			if(last.state == Gear_grabber::Status_detail::State::CLOSING){
				close_timer.update(time,input.enabled);
			} else if(last.state != Gear_grabber::Status_detail::State::CLOSED){
				const Time CLOSE_TIME = .15;//seconds tested
				close_timer.set(CLOSE_TIME);
				last.state = Status_detail::State::CLOSING;
			}
			if(close_timer.done()){
				last.state = Gear_grabber::Status_detail::State::CLOSED;
			}
			break;
		default:
			assert(0);
	}
	last.has_gear = input.has_gear;
}

Gear_grabber::Output control(Gear_grabber::Status status, Gear_grabber::Goal goal){
	Gear_grabber::Output out;
	out.piston = [=](){
		switch(goal){
			case Gear_grabber::Goal::OPEN:
				return Gear_grabber::Output::Piston::OPEN;
			case Gear_grabber::Goal::CLOSE:
				return Gear_grabber::Output::Piston::CLOSE;
			case Gear_grabber::Goal::X:
				switch(status.state){
					case Gear_grabber::Status::State::OPEN:
					case Gear_grabber::Status::State::OPENING:
						return Gear_grabber::Output::Piston::OPEN;
					case Gear_grabber::Status::State::CLOSING:
					case Gear_grabber::Status::State::CLOSED:
						return Gear_grabber::Output::Piston::CLOSE;
					default:
						assert(0);
				}
			default:
				assert(0);
		}
	}();
	out.gear_light = status.has_gear;
	return out;
}

Robot_outputs Gear_grabber::Output_applicator::operator()(Robot_outputs r,Output out)const{
	r.solenoid[PISTON_LOC] = out.piston == Gear_grabber::Output::Piston::OPEN;
	r.driver_station.digital[GEAR_LIGHT_OUT] = out.gear_light; 
	return r;
}

Gear_grabber::Output Gear_grabber::Output_applicator::operator()(Robot_outputs r)const{
	return {r.solenoid[PISTON_LOC] ? Gear_grabber::Output::Piston::OPEN : Gear_grabber::Output::Piston::CLOSE,r.driver_station.digital[GEAR_LIGHT_OUT]};
}

Robot_inputs Gear_grabber::Input_reader::operator()(Robot_inputs r,Input in)const{
	r.digital_io.in[GEAR_SENSOR_LOC] = in.has_gear ? Digital_in::_0 : Digital_in::_1;
	r.robot_mode.enabled = in.enabled;
	return r;
}

Gear_grabber::Input Gear_grabber::Input_reader::operator()(Robot_inputs r)const{
	return {(r.digital_io.in[GEAR_SENSOR_LOC] == Digital_in::_0),r.robot_mode.enabled};
}

Gear_grabber::Status status(Gear_grabber::Status_detail status_detail){
	return status_detail;
}

bool ready(Gear_grabber::Status status,Gear_grabber::Goal goal){
	switch(goal){
		case Gear_grabber::Goal::OPEN:
			return status.state == Gear_grabber::Status::State::OPEN;
		case Gear_grabber::Goal::CLOSE:
			return status.state == Gear_grabber::Status::State::CLOSED;
		default:
			assert(0);
	}
	
	return false;
}

#ifdef GEAR_GRABBER_TEST
#include "formal.h"

int main(){
	{
		Gear_grabber g;
		tester(g);
	}
	{
		cout<<"\n==========================================================\n";
		Gear_grabber g;
		Gear_grabber::Goal goal = Gear_grabber::Goal::CLOSE;
		const bool ENABLED = true;
		bool has_gear = false;
		for(Time t: range(1000)){
			has_gear = t >= 2;
			Gear_grabber::Status_detail status = g.estimator.get();
			Gear_grabber::Output out = control(status,goal);
			
			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";
			
			g.estimator.update(t,Gear_grabber::Input{has_gear,ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing.\n";
				break;
			}
		}

		goal = Gear_grabber::Goal::OPEN;

		for(Time t: range(1000)){
			Gear_grabber::Status_detail status = g.estimator.get();
			Gear_grabber::Output out = control(status,goal);
			
			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";
			
			g.estimator.update(t,Gear_grabber::Input{has_gear,ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing.\n";
				break;
			}
		}
	}
}
#endif
