#include "roller.h"
#include <stdlib.h>
#include <cmath>

using namespace std;

#define ROLLER_ADDRESS 7
#define ROLLER_SPEED 1.0 //TODO tune this

ostream& operator<<(ostream& o, Roller::Goal a){
	#define X(name) if(a==Roller::Goal::name)return o<<"Roller::Goal("#name")";
	X(IN) X(OUT) X(OFF)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o, Roller){ return o<<"Roller()";}

bool operator==(Roller::Output_applicator, Roller::Output_applicator){return 1;}

bool operator==(Roller a, Roller b){ return (a.input_reader==b.input_reader && a.estimator==b.estimator && a.output_applicator==b.output_applicator);}
bool operator!=(Roller a, Roller b){ return !(a==b);}

Robot_outputs Roller::Output_applicator::operator()(Robot_outputs r, Roller::Output out)const{
	switch(out){
		case Roller::Output::OUT:
			r.pwm[ROLLER_ADDRESS]=ROLLER_SPEED;
			break;
		case Roller::Output::OFF:
			r.pwm[ROLLER_ADDRESS]=0.0;	
			break;
		case Roller::Output::IN:
			r.pwm[ROLLER_ADDRESS]=-ROLLER_SPEED;//changed to match the practice bot
			break;
		default:
			assert(0);
	}
	return r;
}

Roller::Output Roller::Output_applicator::operator()(Robot_outputs r)const{
	double pwm = r.pwm[ROLLER_ADDRESS];
	if(pwm < 0.0) return Roller::Output::IN;
	if(pwm == 0.0) return Roller::Output::OFF;
	if(pwm > 0.0) return Roller::Output::OUT;
	assert(0);
}
	
set<Roller::Goal> examples(Roller::Goal*){ 
	return {Roller::Goal::OUT,Roller::Goal::OFF,Roller::Goal::IN};
}

Roller::Output control(Roller::Status_detail, Roller::Goal goal){
	return goal;
}

bool ready(Roller::Status, Roller::Goal){ return 1;}

#ifdef ROLLER_TEST
#include "formal.h"

int main(){
	Roller a;
	tester(a);
}

#endif
