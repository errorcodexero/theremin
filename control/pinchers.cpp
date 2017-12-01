#include "pinchers.h"

using namespace std;

#define PISTON_LOC 3
#define BUCKET_SENSOR 4
#define BUCKET_LIGHT 2

Pinchers::Input::Input():enabled(false),bucket_sensor(false){}
Pinchers::Input::Input(bool a,bool b):enabled(a),bucket_sensor(b){}

Pinchers::Output::Output():piston(Pinchers::Output::Piston::OPEN),bucket_light(false){}
Pinchers::Output::Output(Pinchers::Output::Piston a,bool b):piston(a),bucket_light(b){}

Pinchers::Status_detail::Status_detail():state(Pinchers::Status_detail::State::OPEN),has_bucket(false){}
Pinchers::Status_detail::Status_detail(Pinchers::Status_detail::State a,bool b):state(a),has_bucket(b){}

Pinchers::Estimator::Estimator():last(){
}

std::set<Pinchers::Goal> examples(Pinchers::Goal*){
	return {
		#define X(NAME) Pinchers::Goal::NAME,
		PINCHERS_GOALS(X)
		#undef X
	};
}

std::set<Pinchers::Output::Piston> examples(Pinchers::Output::Piston*){
	return {
		#define X(NAME) Pinchers::Output::Piston::NAME,
		PINCHERS_PISTON_OUTPUTS(X)
		#undef X
	};
}

std::set<Pinchers::Output> examples(Pinchers::Output*){
	set<Pinchers::Output> outs;
	for(Pinchers::Output::Piston a: examples((Pinchers::Output::Piston*)nullptr)){
		outs.insert({a,true});
		outs.insert({a,false});
	}
	return outs;
}

std::set<Pinchers::Input> examples(Pinchers::Input*){
	return {
		{true,true},
		{true,false},
		{false,true},
		{false,false}
	};
}

std::set<Pinchers::Status_detail> examples(Pinchers::Status_detail*){
	set<Pinchers::Status_detail::State> states = {
		#define X(A) Pinchers::Status_detail::State::A,
		PINCHERS_STATES(X)
		#undef X
	};
	set<Pinchers::Status_detail> statuses;
	for(Pinchers::Status_detail::State a: states){
		statuses.insert({a,true});
		statuses.insert({a,false});
	}
	return statuses;
}

std::ostream& operator<<(std::ostream& o,Pinchers::Goal g){
	#define X(name) if(g==Pinchers::Goal::name) return o<<""#name;
	PINCHERS_GOALS(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Pinchers::Output::Piston a){
	#define X(A) if(a==Pinchers::Output::Piston::A) return o<<""#A;
	PINCHERS_PISTON_OUTPUTS(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Pinchers::Output a){
	o<<"Output(";
	o<<"piston:"<<a.piston;
	o<<" bucket_light:"<<a.bucket_light;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Pinchers::Input a){
	o<<"Input(";
	o<<"enabled:"<<a.enabled;
	o<<" bucket_sensor:"<<a.bucket_sensor;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Pinchers::Status_detail::State a){
	#define X(STATUS) if(a==Pinchers::Status_detail::State::STATUS) return o<<""#STATUS;
	PINCHERS_STATES(X)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Pinchers::Status_detail a){
	o<<"Status_detail(";
	o<<"state:"<<a.state;
	o<<" has_bucket:"<<a.has_bucket;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Pinchers const&){
	return o<<"Pinchers()";
}

bool operator<(Pinchers::Output a,Pinchers::Output b){
	if(a.piston < b.piston) return true;
	if(b.piston < a.piston) return false;
	return !a.bucket_light && b.bucket_light;
}
bool operator==(Pinchers::Output a,Pinchers::Output b){
	return a.piston == b.piston && a.bucket_light == b.bucket_light;
}
bool operator!=(Pinchers::Output a,Pinchers::Output b){ return !(a==b); }

bool operator<(Pinchers::Input a,Pinchers::Input b){ 
	if(b.enabled && !a.enabled) return true;
	if(a.enabled && !b.enabled) return false;
	return !a.bucket_sensor && b.bucket_sensor;
}
bool operator==(Pinchers::Input a,Pinchers::Input b){
	return a.enabled == b.enabled && a.bucket_sensor == b.bucket_sensor;
}
bool operator!=(Pinchers::Input a, Pinchers::Input b){ return !(a==b); }

bool operator<(Pinchers::Status_detail a,Pinchers::Status_detail b){
	if(a.state < b.state) return true;
	if(b.state < a.state) return false;
	return !a.has_bucket && b.has_bucket;
}
bool operator==(Pinchers::Status_detail a,Pinchers::Status_detail b){
	return a.state == b.state && a.has_bucket == b.has_bucket;
}
bool operator!=(Pinchers::Status_detail a,Pinchers::Status_detail b){ return !(a==b); }

bool operator==(Pinchers::Estimator,Pinchers::Estimator){ return 1; }
bool operator!=(Pinchers::Estimator a, Pinchers::Estimator b){ return !(a==b); }
ostream& operator<<(ostream& o, Pinchers::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" state_timer:"<<a.state_timer<<")";
}

bool operator==(Pinchers,Pinchers){ return 1; }
bool operator!=(Pinchers a, Pinchers b){ return !(a==b); }

Pinchers::Input Pinchers::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.robot_mode.enabled,r.digital_io.in[BUCKET_SENSOR]==Digital_in::_0};
}

Robot_inputs Pinchers::Input_reader::operator()(Robot_inputs r, Pinchers::Input in) const{
	r.robot_mode.enabled = in.enabled;
	r.digital_io.in[BUCKET_SENSOR] = in.bucket_sensor ? Digital_in::_0 : Digital_in::_1;
	return r;
}

Robot_outputs Pinchers::Output_applicator::operator()(Robot_outputs r, Pinchers::Output o)const{
	r.solenoid[PISTON_LOC] = o.piston == Pinchers::Output::Piston::OPEN;
	r.driver_station.digital[BUCKET_LIGHT] = o.bucket_light;
	return r;
}

Pinchers::Output Pinchers::Output_applicator::operator()(Robot_outputs const& r)const{
	return {r.solenoid[PISTON_LOC] ? Output::Piston::OPEN : Output::Piston::CLOSE,r.driver_station.digital[BUCKET_LIGHT]};
}

void Pinchers::Estimator::update(Time time,Pinchers::Input input,Pinchers::Output output){
	switch(output.piston){
		case Pinchers::Output::Piston::CLOSE:
			if(last.state == Status::State::CLOSING){
				state_timer.update(time,input.enabled);
			} else if(last.state != Status::State::CLOSED){ 
				const Time CLOSE_TIME = 1.0;//seconds. assumed
				last.state = Status::State::CLOSING;
				state_timer.set(CLOSE_TIME);
			}
			if(state_timer.done() || last.state == Status::State::CLOSED) {
				last.state = Status::State::CLOSED;
			}
			break;
		case Pinchers::Output::Piston::OPEN:
			if(last.state == Status::State::OPENING){
				state_timer.update(time,input.enabled);
			} else if(last.state != Status::State::OPEN){ 
				const Time OPEN_TIME = 2.2;//seconds. assumed
				last.state = Status::State::OPENING;
				state_timer.set(OPEN_TIME);
			}
			if(state_timer.done() || last.state == Status::State::OPEN) { 
				last.state = Status::State::OPEN;
			}
			break;
		default:
			assert(0);
	}
	last.has_bucket = input.bucket_sensor;
}

Pinchers::Status Pinchers::Estimator::get()const{
	return last;
}

Pinchers::Output control(Pinchers::Status status,Pinchers::Goal goal){
	Pinchers::Output out;
	out.piston = [=](){
		switch(goal){
			case Pinchers::Goal::CLOSE:
				return Pinchers::Output::Piston::CLOSE;
			case Pinchers::Goal::OPEN:
				return Pinchers::Output::Piston::OPEN;
			case Pinchers::Goal::X:
				switch(status.state){
					case Pinchers::Status::State::OPEN:
					case Pinchers::Status::State::OPENING:
						return Pinchers::Output::Piston::OPEN;
					case Pinchers::Status::State::CLOSING:
					case Pinchers::Status::State::CLOSED:
						return Pinchers::Output::Piston::CLOSE;
					default:
						assert(0);
				}
			default:
				assert(0);
		}
	}();
	out.bucket_light = status.has_bucket;
	return out;
}

Pinchers::Status status(Pinchers::Status s){
	return s;
}

bool ready(Pinchers::Status status,Pinchers::Goal goal){
	switch(goal){
		case Pinchers::Goal::OPEN:
			return status.state == Pinchers::Status::State::OPEN;
		case Pinchers::Goal::CLOSE:
			return status.state == Pinchers::Status::State::CLOSED;
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

			a.estimator.update(t,Pinchers::Input{ENABLED,false},out);
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

			a.estimator.update(t,Pinchers::Input{ENABLED,false},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}

		}
	}
}
#endif
