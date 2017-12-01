#include "panel.h"
#include <iostream>
#include <stdlib.h> 
#include "util.h"
#include "../util/util.h"
#include <cmath>

using namespace std;

static const unsigned int AUTO_SELECTOR_AXIS=0,GRABBER_AXIS=1, PINCHERS_AXIS=2;
static const unsigned int GRABBER_TOGGLE_LOC=0;

#define BUTTONS \
	X(grabber_toggle)
#define THREE_POS_SWITCHES \
	X(grabber) X(pinchers) 
#define TEN_POS_SWITCHES \
	X(auto_select)
#define DIALS \

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
	grabber(Grabber::AUTO),
	pinchers(Pinchers::AUTO),
	auto_select(0)	
{}

ostream& operator<<(ostream& o,Panel::Grabber a){
	#define X(NAME) if(a==Panel::Grabber::NAME) return o<<""#NAME;
	X(UP) X(DOWN) X(AUTO)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Panel::Pinchers a){
	#define X(NAME) if(a==Panel::Pinchers::NAME) return o<<""#NAME;
	X(OPEN) X(CLOSE) X(AUTO)
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
		p.auto_select = interpret_10_turn_pot(auto_dial_value);
	}
	{//two position switches
	}
	{//three position switches

		float grabber = d.axis[GRABBER_AXIS];
		p.grabber = [&]{
			static const float UP=-1,DOWN=0,AUTO=1;
			if(set_button(grabber,UP,DOWN,AUTO)) return Panel::Grabber::DOWN;
			if(set_button(grabber,DOWN,AUTO,ARTIFICIAL_MAX)) return Panel::Grabber::AUTO;
			return Panel::Grabber::UP;
		}();

		float pinchers = d.axis[PINCHERS_AXIS];
		p.pinchers = [&]{
			static const float OPEN=-1,CLOSE=0,AUTO=1;
			if(set_button(pinchers,OPEN,CLOSE,AUTO)) return Panel::Pinchers::CLOSE;
			if(set_button(pinchers,CLOSE,AUTO,ARTIFICIAL_MAX)) return Panel::Pinchers::AUTO;
			return Panel::Pinchers::OPEN;
		}();

	}	
	{//buttons
		p.grabber_toggle=d.button[GRABBER_TOGGLE_LOC];
	}
	{//Dials
	}
	return p;
}

Panel interpret_gamepad(Joystick_data d){
	Panel p;
	p.in_use = get_in_use(d);
	if(!p.in_use) return p;
	
	p.grabber_toggle = d.button[Gamepad_button::A];
	
	p.pinchers = Panel::Pinchers::AUTO;
	if(d.button[Gamepad_button::LB]) p.pinchers = Panel::Pinchers::CLOSE;
	if(d.button[Gamepad_button::RB]) p.pinchers = Panel::Pinchers::OPEN;

	p.grabber = Panel::Grabber::AUTO;
	switch(pov_section(d.pov)){
		case POV_section::CENTER:
			break;
		case POV_section::UP:
			p.grabber = Panel::Grabber::UP;
			break;
		case POV_section::UP_LEFT:
			break;
		case POV_section::LEFT:
			break;
		case POV_section::DOWN_LEFT:
			break;
		case POV_section::DOWN:
			p.grabber = Panel::Grabber::DOWN;
			break;
		case POV_section::DOWN_RIGHT:
			break;
		case POV_section::RIGHT:
			break;
		case POV_section::UP_RIGHT:
			break;
		default:
		assert(0);
	}

	p.auto_select=0;
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
