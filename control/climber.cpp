#include "climber.h"

static const int CLIMBER_PWM = 8, CLIMBER_CAN = 0; //actuall one, but we account for the fact that CAN doesn't start at zero in talon_srx_controls. this is in trello as a todo
static const double CLIMB_POWER = 1.0;

using namespace std;

set<Climber::Output> examples(Climber::Goal*){
	return {Climber::Output::RELEASE, Climber::Output::STOP, Climber::Output::STANDARD_CLIMB, Climber::Output::TURBO_CLIMB};
}

ostream& operator<<(ostream& o, Climber::Goal a){
	#define X(G) if(a==Climber::Goal::G) return o<<""#G;
	X(STANDARD_CLIMB)
	X(TURBO_CLIMB)
	X(STOP)
	X(RELEASE)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Climber const&){
	return o<<"Climber()";
}

bool operator==(Climber,Climber){ return 1; }
bool operator!=(Climber a, Climber b){ return !(a==b); }

Robot_outputs Climber::Output_applicator::operator()(Robot_outputs r, Climber::Output o)const{
	r.talon_srx[CLIMBER_CAN] = Talon_srx_output::percent([=]{
		switch(o){
			case Climber::Output::RELEASE:
				return CLIMB_POWER * 0.5;
			case Climber::Output::STANDARD_CLIMB:
				return -CLIMB_POWER * 0.5;
			case Climber::Output::TURBO_CLIMB:
				return -CLIMB_POWER;
			case Climber::Output::STOP:
				return 0.0;
			default:
				assert(0);
		}

	}());
	r.pwm[CLIMBER_PWM] = [=]{
		switch(o){
			case Climber::Output::RELEASE:
				return CLIMB_POWER * 0.5;
			case Climber::Output::STANDARD_CLIMB:
				return -CLIMB_POWER * 0.5;
			case Climber::Output::TURBO_CLIMB:
				return -CLIMB_POWER;
			case Climber::Output::STOP:
				return 0.0;
			default:
				assert(0);
		}
	}();
	return r;
}

Climber::Output Climber::Output_applicator::operator()(Robot_outputs const& r)const{
	double talon_p = r.talon_srx[CLIMBER_CAN].power_level, pwm_p = r.pwm[CLIMBER_PWM];
	
	static const double TOL = 0.01;	

	if(talon_p < -CLIMB_POWER + TOL && pwm_p < -CLIMB_POWER + TOL) return Climber::Output::TURBO_CLIMB;
	if(talon_p < -CLIMB_POWER * 0.5 + TOL && pwm_p < -CLIMB_POWER * 0.5 + TOL) return Climber::Output::STANDARD_CLIMB;
	if(talon_p > 0 && pwm_p > 0) return Climber::Output::RELEASE;
	return Climber::Output::STOP;
}

Climber::Output control(Climber::Status,Climber::Goal goal){
	return goal;
}

bool ready(Climber::Status,Climber::Goal){
	return true;
}

#ifdef CLIMBER_TEST
#include "formal.h"
int main(){
	Climber c;
	tester(c);
}
#endif
