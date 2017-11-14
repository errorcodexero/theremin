#include "dozer.h"
#include "stdlib.h"

#define INOUT_ADDRESS 0 //TODO

std::ostream& operator<<(std::ostream& o,Dozer::Goal a){ 
	#define X(NAME) if(a==Dozer::Goal::NAME) return o<<""#NAME")";
	X(IN) X(STOP) X(OUT)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Dozer::Input a){
	o<<"(";
	o<<a.enabled;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Dozer::Status_detail a){
	#define X(NAME) if(a==Dozer::Status_detail::NAME) return o<<""#NAME")";
	X(IN) X(OUT) X(UNKNOWN) X(PROBABLY_OUT) X(PROBABLY_IN)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Dozer::Estimator a){
	return o<<"(last:"<<a.last<<" state_timer:"<<a.state_timer<<" last_output:"<<a.last_output<<")";
}

std::ostream& operator<<(std::ostream& o,Dozer a){
	return o<<"Dozer("<<a.estimator<<")";
}

bool operator==(Dozer::Input a,Dozer::Input b){
	return a.enabled==b.enabled;
}

bool operator!=(Dozer::Input a,Dozer::Input b){
	return !(a==b);
}

bool operator<(Dozer::Input a,Dozer::Input b){
	return a.enabled<b.enabled;
}

bool operator==(Dozer::Input_reader,Dozer::Input_reader){
	return true;
}

bool operator<(Dozer::Input_reader,Dozer::Input_reader){
	return false;
}

bool operator==(Dozer::Estimator a,Dozer::Estimator b){
	return a.last==b.last && a.state_timer==b.state_timer && a.last_output==b.last_output;
}

bool operator!=(Dozer::Estimator a,Dozer::Estimator b){
	return !(a==b);
}

bool operator==(Dozer::Output_applicator,Dozer::Output_applicator){
	return true;
}

bool operator==(Dozer a,Dozer b){
	return (a.input_reader==b.input_reader && a.estimator==b.estimator && a.output_applicator==b.output_applicator);
}

bool operator!=(Dozer a,Dozer b){
	return !(a==b);
}

Dozer::Input Dozer::Input_reader::operator()(Robot_inputs const& a)const{
	return {a.robot_mode.enabled};
}

Robot_inputs Dozer::Input_reader::operator()(Robot_inputs r,Dozer::Input a)const{
	r.robot_mode.enabled=a.enabled;
	return r;
}

Dozer::Output Dozer::Output_applicator::operator()(Robot_outputs r)const{
	auto v=r.relay[INOUT_ADDRESS];
	return (v==Relay_output::_01? Dozer::Output::OUT : (v==Relay_output::_10? Dozer::Output::IN : Dozer::Output::STOP));
}

Robot_outputs Dozer::Output_applicator::operator()(Robot_outputs r,Dozer::Output out)const{
	r.relay[INOUT_ADDRESS]=[&]{
		switch(out){
			case Dozer::Output::IN: return Relay_output::_10;
			case Dozer::Output::OUT: return Relay_output::_01;
			case Dozer::Output::STOP: return Relay_output::_00;
			default: assert(0);
		}
	}();
	return r;
}

Dozer::Estimator::Estimator():last(Dozer::Status_detail::UNKNOWN),state_timer(),last_output(Dozer::Output::STOP){}

Dozer::Status_detail Dozer::Estimator::get()const{
	return last;
}

void Dozer::Estimator::update(Time time,Dozer::Input in,Dozer::Output output){
	state_timer.update(time,in.enabled);
	refresh_timer.update(time,in.enabled);
	static const double MOVE_TIME=1;//sec
	static const double REFRESH=15;//sec
	static const double PULSE=1;//sec
	switch(last){
		case Status::IN:
			if(output==Output::OUT){
				last=Status::UNKNOWN;
				state_timer.set(MOVE_TIME);
			} else if(output==Output::STOP && last_output==Output::IN){
				refresh_timer.set(REFRESH);
			}
			if(refresh_timer.done()){
				last=Status::PROBABLY_IN;
			}
			break;
		case Status::OUT:
			if(output==Output::IN){
				last=Status::UNKNOWN;
				state_timer.set(MOVE_TIME);
			} else if(output==Output::STOP && last_output==Output::OUT){
				refresh_timer.set(REFRESH);
			}
			if(refresh_timer.done()){
				last=Status::PROBABLY_OUT;
			}
			break;
		case Status::PROBABLY_OUT:
			if(output==Output::OUT){
				last=Status::UNKNOWN;
				state_timer.set(PULSE);
			}	
			break;
		case Status::PROBABLY_IN:
			if(output==Output::IN){
				last=Status::UNKNOWN;
				state_timer.set(PULSE);
			}
			break;
		case Status::UNKNOWN:
			if(output!=last_output){
				state_timer.set(MOVE_TIME);
			}
			if(state_timer.done()){
				switch(output){
					case Output::IN:
						last=Status::IN;
						break;
					case Output::OUT:
						last=Status::OUT;
						break;
					case Output::STOP:
						break;
					default: assert(0);
				}
			}
			break;
		default:
			assert(0);
	}

	last_output=output;
}

std::set<Dozer::Input> examples(Dozer::Input*){
	return {{0},{1}};
}

std::set<Dozer::Goal> examples(Dozer::Goal*){
	return {Dozer::Goal::IN,Dozer::Goal::STOP,Dozer::Goal::OUT};
}

std::set<Dozer::Status_detail> examples(Dozer::Status_detail*){
	return {Dozer::Status_detail::IN,Dozer::Status_detail::OUT,Dozer::Status_detail::PROBABLY_OUT,Dozer::Status_detail::PROBABLY_IN,Dozer::Status_detail::UNKNOWN};
}

Dozer::Output control(Dozer::Status_detail status,Dozer::Goal goal){
	switch(goal){
		case Dozer::Goal::IN:
			if(status==Dozer::Status_detail::IN) return Dozer::Output::STOP;
			return Dozer::Output::IN;
		case Dozer::Goal::OUT:
			if(status==Dozer::Status_detail::OUT) return Dozer::Output::STOP;
			return Dozer::Output::OUT;
		case Dozer::Goal::STOP:
			return Dozer::Output::STOP;
		default: assert(0);
	}
}

Dozer::Status status(Dozer::Status_detail a){ return a; }

bool ready(Dozer::Status status,Dozer::Goal goal){
	switch(goal){
		case Dozer::Goal::IN:
			return status==Dozer::Status::IN;
		case Dozer::Goal::OUT:
			return status==Dozer::Status::OUT;
		case Dozer::Goal::STOP:
			return true;
		default:
			assert(0);
	}
	return true;
}

#ifdef DOZER_TEST
#include "formal.h"

int main(){
	Dozer a;
	tester(a);
}

#endif
