#include "panel.h"
#include <iostream>
#include <stdlib.h> 
#include "util.h"
#include "../util/util.h"
#include <cmath>

using namespace std;
static const unsigned int ROLLER_CONTROL_AXIS=0, ROLLER_AXIS=1, ROLLER_ARM_AXIS=2, CLIMBER_MODE_AXIS=3, GEAR_GRABBER_AXIS=4, GEAR_ARM_AXIS=5, AUTO_SELECTOR_AXIS=6, SPEED_DIAL_AXIS=7;//TODO: rename constants
static const unsigned int CAMERA_LIGHT_LOC=1, SHOOT_LOC=2, PREP_COLLECT_GEAR_LOC=3, PREP_SCORE_GEAR_LOC=4, COLLECT_GEAR_LOC=5, SCORE_GEAR_LOC=6, CLIMB_LOC=7, LEARN_LOC=8, GEAR_SENSING_FULL_AUTO_LOC=9, GEAR_SENSING_NO_AUTO_LOC=10;//TODO: rename constants 

#define BUTTONS \
	X(camera_light) X(shoot) X(gear_prep_collect) X(gear_prep_score) X(gear_collect) X(gear_score) X(climb) X(learn)
#define THREE_POS_SWITCHES \
	X(roller_control) X(roller) X(roller_arm) X(climber_mode) X(gear_grabber) X(gear_arm) X(gear_sensing)
#define TEN_POS_SWITCHES \
	X(auto_select)
#define DIALS \
	X(speed_dial)

#define PANEL_ITEMS \
	BUTTONS \
	THREE_POS_SWITCHES \
	TEN_POS_SWITCHES \
	DIALS

Panel::Panel():
	in_use(0),
	#define X(BUTTON) BUTTON(false),
	BUTTONS
	#undef X
	roller_control(Roller_control::AUTO),
	roller(Roller::AUTO),
	roller_arm(Roller_arm::AUTO),
	climber_mode(Climber_mode::STANDARD),
	gear_grabber(Gear_grabber::CLOSED),
	gear_arm(Gear_arm::DOWN),
	gear_sensing(Gear_sensing::FULL_AUTO),
	auto_select(0),
	speed_dial(0)
{}

ostream& operator<<(ostream& o,Panel::Roller_control a){
	#define X(NAME) if(a==Panel::Roller_control::NAME) return o<<""#NAME;
	X(OFF) X(AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Roller a){
	#define X(NAME) if(a==Panel::Roller::NAME) return o<<""#NAME;
	X(OUT) X(IN) X(AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Roller_arm a){
	#define X(NAME) if(a==Panel::Roller_arm::NAME) return o<<""#NAME;
	X(STOW) X(LOW) X(AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Climber_mode a){
	#define X(NAME) if(a==Panel::Climber_mode::NAME) return o<<""#NAME;
	X(TURBO) X(STANDARD) X(RELEASE)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Gear_grabber a){
	#define X(NAME) if(a==Panel::Gear_grabber::NAME) return o<<""#NAME;
	X(OPEN) X(CLOSED) X(AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Gear_arm a){
	#define X(NAME) if(a==Panel::Gear_arm::NAME) return o<<""#NAME;
	X(UP) X(DOWN) X(AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Gear_sensing a){
	#define X(NAME) if(a==Panel::Gear_sensing::NAME) return o<<""#NAME;
	X(NO_AUTO) X(SEMI_AUTO) X(FULL_AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel p){
	o<<"Panel(";
	o<<"in_use:"<<p.in_use;
	#define X(NAME) o<<", "#NAME":"<<p.NAME;
	PANEL_ITEMS
	#undef X
	return o<<")";
}

bool operator==(Panel const& a,Panel const& b){
	return true
	#define X(NAME) && a.NAME==b.NAME
	PANEL_ITEMS
	#undef X
	;
}

bool operator!=(Panel const& a,Panel const& b){
	return !(a==b);
}

float axis_to_percent(double a){
	return .5-(a/2);
}

bool set_button(const float AXIS_VALUE, const float LOWER_VALUE, const float TESTING_VALUE, const float UPPER_VALUE){
	float lower_tolerance = (TESTING_VALUE - LOWER_VALUE)/2;
	float upper_tolerance = (UPPER_VALUE - TESTING_VALUE)/2;
	float min = TESTING_VALUE - lower_tolerance;
	float max = TESTING_VALUE + upper_tolerance; 
	return (AXIS_VALUE > min && AXIS_VALUE < max);
}

bool get_in_use(Joystick_data d){
	for(int i = 0; i < JOY_AXES; i++) {
		if(d.axis[i] != 0) return true;
	}
	for(int i = 0; i < JOY_BUTTONS; i++) {
		if(d.button[i] != 0) return true;
	}
	return false;
}

Panel interpret_oi(Joystick_data d){
	Panel p;
	static const float ARTIFICIAL_MAX = 1.5;
	{
		p.in_use=get_in_use(d);
		if(!p.in_use) return p;
	}
	{//set the auto mode number from the dial value
		float auto_dial_value = d.axis[AUTO_SELECTOR_AXIS];
		p.auto_select = interpret_20_turn_pot(auto_dial_value);
	}
	{//two position switches
	}
	{//three position switches
		float roller_control = d.axis[ROLLER_CONTROL_AXIS];
		p.roller_control = [&]{
			static const float AUTO=-1,OFF1=0,OFF2=1;
			if(set_button(roller_control,AUTO,OFF1,OFF2)) return Panel::Roller_control::OFF;
			if(set_button(roller_control,OFF1,OFF2,ARTIFICIAL_MAX)) return Panel::Roller_control::OFF;
			return Panel::Roller_control::AUTO;
		}();
		
		float roller = d.axis[ROLLER_AXIS];
		p.roller = [&]{
			static const float AUTO=-1,IN=0,OUT=1;
			if(set_button(roller,AUTO,IN,OUT)) return Panel::Roller::IN;
			if(set_button(roller,IN,OUT,ARTIFICIAL_MAX)) return Panel::Roller::OUT;
			return Panel::Roller::AUTO;
		}();

		float roller_arm = d.axis[ROLLER_ARM_AXIS];
		p.roller_arm = [&]{
			static const float AUTO=-1,LOW=0,STOW=1;
			if(set_button(roller_arm,AUTO,LOW,STOW)) return Panel::Roller_arm::LOW;
			if(set_button(roller_arm,LOW,STOW,ARTIFICIAL_MAX)) return Panel::Roller_arm::STOW;
			return Panel::Roller_arm::AUTO;
		}();

		float climber_mode = d.axis[CLIMBER_MODE_AXIS];
		p.climber_mode = [&]{
			static const float STANDARD=-1,TURBO=0,RELEASE=1;
			if(set_button(climber_mode,STANDARD,TURBO,RELEASE)) return Panel::Climber_mode::TURBO;
			if(set_button(climber_mode,TURBO,RELEASE,ARTIFICIAL_MAX)) return Panel::Climber_mode::RELEASE;
			return Panel::Climber_mode::STANDARD;
		}();

		float gear_grabber = d.axis[GEAR_GRABBER_AXIS];
		p.gear_grabber = [&]{
			static const float AUTO=-1,CLOSED=0,OPEN=1;
			if(set_button(gear_grabber,AUTO,CLOSED,OPEN)) return Panel::Gear_grabber::CLOSED;
			if(set_button(gear_grabber,CLOSED,OPEN,ARTIFICIAL_MAX)) return Panel::Gear_grabber::OPEN;
			return Panel::Gear_grabber::AUTO;
		}();

		float gear_arm = d.axis[GEAR_ARM_AXIS];
		p.gear_arm = [&]{
			static const float AUTO=-1,DOWN=0,UP=1;
			if(set_button(gear_arm,AUTO,DOWN,UP)) return Panel::Gear_arm::DOWN;
			if(set_button(gear_arm,DOWN,UP,ARTIFICIAL_MAX)) return Panel::Gear_arm::UP;
			return Panel::Gear_arm::AUTO;
		}();
		
		bool gear_sensing_full_auto = d.button[GEAR_SENSING_FULL_AUTO_LOC], gear_sensing_no_auto = d.button[GEAR_SENSING_NO_AUTO_LOC];
		p.gear_sensing = [&]{
			if(gear_sensing_full_auto) return Panel::Gear_sensing::FULL_AUTO;
			if(gear_sensing_no_auto) return Panel::Gear_sensing::NO_AUTO;
			return Panel::Gear_sensing::SEMI_AUTO;
		}();
	
	}	
	{//buttons
		p.camera_light=d.button[CAMERA_LIGHT_LOC];
		p.shoot=d.button[SHOOT_LOC];
		p.gear_prep_collect=d.button[PREP_COLLECT_GEAR_LOC];
		p.gear_prep_score=d.button[PREP_SCORE_GEAR_LOC];
		p.gear_collect=d.button[COLLECT_GEAR_LOC];
		p.gear_score=d.button[SCORE_GEAR_LOC];
		p.climb=d.button[CLIMB_LOC];
		p.learn=d.button[LEARN_LOC];
	}
	{//Dials
		p.speed_dial = -d.axis[SPEED_DIAL_AXIS];
	}
	return p;
}

Panel interpret_test_oi(Joystick_data d){	
	//static const unsigned int POTENTIOMETER_AXIS=1, TEN_POS_SWITCH_AXIS = 0; //TODO: need real values
	//static const unsigned int BUTTON_0_LOC=0, BUTTON_1_LOC=1, BUTTON_2_LOC=2, BUTTON_3_LOC=3, TWO_POS_SWITCH_0_LOC = 4, TWO_POS_SWITCH_1_LOC =5; //TODO: need real values

	Panel p;
	{
		p.in_use=[&](){
			for(int i=0;i<JOY_AXES;i++) {
				if(d.axis[i]!=0)return true;
			}
			for(int i=0;i<JOY_BUTTONS;i++) {
				if(d.button[i]!=0)return true;
			}
			return false;
		}();
		if(!p.in_use) return p;
	}
	//p. = interpret_10_turn_pot(d.axis[TEN_POS_SWITCH_AXIS]); //set the switch value from the pot value
	{//two position switches
		//p. = d.button[TWO_POS_SWITCH_0_LOC];
		//p. = d.button[TWO_POS_SWITCH_1_LOC];
	}
	{//buttons
		//p. = d.button[BUTTON_0_LOC];
		//p. = d.button[BUTTON_1_LOC];
		//p. = d.button[BUTTON_2_LOC];
		//p. = d.button[BUTTON_3_LOC];
	}
	{//dials
		//p. = d.axis[POTENTIOMETER_AXIS];
	}
	return p;
}

Panel interpret_gamepad(Joystick_data d){
	Panel p;
	p.in_use = get_in_use(d);
	if(!p.in_use) return p;
	
	//TODO: Add in all of the new controls

	bool alternative_op = d.button[Gamepad_button::LB];
	p.auto_select=0;
	p.speed_dial = (d.axis[Gamepad_axis::LTRIGGER]-.5)*2;
	p.learn = d.button[Gamepad_button::START];

	if(!alternative_op){
		p.gear_prep_score = d.button[Gamepad_button::Y];
		p.gear_score = d.button[Gamepad_button::B];
		p.gear_collect = d.button[Gamepad_button::X];
		p.gear_prep_collect = d.button[Gamepad_button::A];

		cout<<"\nPOV:"<<d.pov<<"\n";

		switch(pov_section(d.pov)){
			case POV_section::CENTER:
				break;
			case POV_section::UP:
				break;
			case POV_section::UP_LEFT:
				break;
			case POV_section::LEFT:
				p.camera_light=true;
				break;
			case POV_section::DOWN_LEFT:
				break;
			case POV_section::DOWN:
				p.climb = true;
				break;
			case POV_section::DOWN_RIGHT:
				break;
			case POV_section::RIGHT:
				p.shoot = true;
				break;
			case POV_section::UP_RIGHT:
				break;
			default:
				assert(0);
		}

		switch(joystick_section(d.axis[Gamepad_axis::LEFTX],d.axis[Gamepad_axis::LEFTY])){	
			case Joystick_section::UP:
				break;
			case Joystick_section::RIGHT:
				p.climber_mode=Panel::Climber_mode::STANDARD;
				break;
			case Joystick_section::DOWN:
				p.climber_mode=Panel::Climber_mode::TURBO;
				break;
			case Joystick_section::LEFT:
				p.climber_mode=Panel::Climber_mode::RELEASE;
				break;
			case Joystick_section::CENTER:
				break;
			default:
				assert(0);
		}

		p.roller_control=Panel::Roller_control::AUTO;
		p.roller=Panel::Roller::AUTO;
		p.roller_arm=Panel::Roller_arm::AUTO;
		p.gear_grabber=Panel::Gear_grabber::AUTO;
		p.gear_arm=Panel::Gear_arm::AUTO;
		p.gear_sensing=Panel::Gear_sensing::SEMI_AUTO;
	} else {
		p.roller_control=d.button[Gamepad_button::RB]?Panel::Roller_control::OFF:Panel::Roller_control::AUTO;
		p.climber_mode=Panel::Climber_mode::STANDARD;

		if(d.button[Gamepad_button::B]) p.gear_grabber=Panel::Gear_grabber::CLOSED;
		else if(!d.button[Gamepad_button::X]) p.gear_grabber= Panel::Gear_grabber::OPEN;
		else p.gear_grabber=Panel::Gear_grabber::AUTO;

		if(d.button[Gamepad_button::Y]) p.gear_arm=Panel::Gear_arm::UP;
		else if(!d.button[Gamepad_button::A]) p.gear_arm=Panel::Gear_arm::DOWN;
		else p.gear_arm=Panel::Gear_arm::AUTO;

		p.roller=Panel::Roller::AUTO;
		switch(pov_section(d.pov)){
			case POV_section::CENTER:
				break;
			case POV_section::UP:
				break;
			case POV_section::UP_LEFT:
				break;
			case POV_section::LEFT:
				p.roller=Panel::Roller::OUT;
				break;
			case POV_section::DOWN_LEFT:
				break;
			case POV_section::DOWN:
				break;
			case POV_section::DOWN_RIGHT:
				break;
			case POV_section::RIGHT:
				p.roller=Panel::Roller::IN;
				break;
			case POV_section::UP_RIGHT:
				break;
			default:
				assert(0);
		}
		switch(joystick_section(d.axis[Gamepad_axis::RIGHTX],d.axis[Gamepad_axis::RIGHTY])){
			case Joystick_section::UP:
				p.roller_arm=Panel::Roller_arm::STOW;
				break;
			case Joystick_section::LEFT:
				break;
			case Joystick_section::DOWN:
				p.roller_arm=Panel::Roller_arm::LOW;
				break;
			case Joystick_section::RIGHT:
				break;
			case Joystick_section::CENTER:
				p.roller_arm=Panel::Roller_arm::AUTO;
				break;
			default:
				assert(0);
		}
	}

	return p;
}

Joystick_data driver_station_input_rand(){
	Joystick_data r;
	for(unsigned i=0;i<JOY_AXES;i++){
		r.axis[i]=(0.0+rand()%101)/100;
	}
	for(unsigned i=0;i<JOY_BUTTONS;i++){
		r.button[i]=rand()%2;
	}
	return r;
}

Panel rand(Panel*){
	return interpret_oi(driver_station_input_rand());
}

#ifdef PANEL_TEST
int main(){
	Panel p;
	for(unsigned i=0;i<50;i++){
		interpret_oi(driver_station_input_rand());
	}
	cout<<p<<"\n";
	return 0;
}
#endif
