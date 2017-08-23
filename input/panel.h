#ifndef PANEL_H
#define PANEL_H 

#include "../util/maybe.h"
#include "../util/interface.h"

struct Panel{
	static const unsigned PORT = 2;
	bool in_use;
	//*Buttons:
	bool camera_light;
	bool shoot;
	bool gear_prep_collect;
	bool gear_prep_score;
	bool gear_collect;
	bool gear_score;
	bool climb;
	bool learn;
	//2 position swicthes:
	//3 position switches:
	enum class Roller_control{OFF,AUTO};
	Roller_control roller_control;
	enum class Roller{OUT,IN,AUTO};
	Roller roller;
	enum class Roller_arm{STOW,LOW,AUTO};
	Roller_arm roller_arm;
	enum class Climber_mode{TURBO,STANDARD,RELEASE};
	Climber_mode climber_mode;
	enum class Gear_grabber{OPEN,CLOSED,AUTO};
	Gear_grabber gear_grabber;
	enum class Gear_arm{UP,DOWN,AUTO};
	Gear_arm gear_arm;
	enum class Gear_sensing{NO_AUTO,SEMI_AUTO,FULL_AUTO};
	Gear_sensing gear_sensing;
	//10 *position switches:
	int auto_select;//0-19
	//Dials:
	float speed_dial;//can't read dial
	Panel();
};

bool operator!=(Panel const&,Panel const&);
std::ostream& operator<<(std::ostream&,Panel::Gear_sensing);
std::ostream& operator<<(std::ostream&,Panel);

Panel interpret_oi(Joystick_data);
Panel interpret_test_oi(Joystick_data);
Panel interpret_gamepad(Joystick_data);

Joystick_data driver_station_input_rand();
Panel rand(Panel*);

#endif
