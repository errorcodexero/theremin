#include "autonomous.h"

#include "teleop.h" 
#include "chain.h"
#include "step.h"
#include "align.h"
using namespace std; 

double deg_to_rad(double deg){
	return deg/180*PI;
}

const Inch BUMPER_LENGTH = 3.0;//inches thickness of one bumper

const Inch ROBOT_LENGTH = 28.0 + 1 * BUMPER_LENGTH; //inches from front to back //TODO: change to match number of sides with bumpers

Executive make_test_step(auto a){
	return Executive{Chain{
		Step{a},
		Executive{Teleop{}}
	}};
}

Executive get_auto_mode(Next_mode_info info){
	if(!info.autonomous) return Executive{Teleop()};

	////////////////////////////
	//
	// Parts of other autonomous modes
	// 

	//for when just want to run across the field at the end of autonomous
	static const Inch DASH_DIST = 20*12;
	const Executive dash{Chain{
		Step{Drive_straight{DASH_DIST}},
		Executive{Teleop{}}
	}};

	static const Inch EXTENDED_GEAR_LENGTH = 7.6;//how far the gear extends out of the robot when deployed outward
	static const Inch ALIGN_DIST = 12;//decreases distance from the drive distance to compinsate for the fact that vision doesnt work well at close distances
	//////////////////////////
	//
	// Full autonomous modes
	//
	
	//Auto mode which does nothing
	const Executive auto_null{Teleop{}};

	//Auto mode for crossing the baseline
	static const Inch BASELINE_DIST = 7 * 12 + 9.25;//distance from baseline to alliance wall // from testing
	Executive auto_baseline{Chain{
		Step{Drive_straight{BASELINE_DIST + 12}},//move a little farther to give us some room for error
		Executive{Teleop{}}
	}};

	//Scores a gear on the middle peg and then stops
	static const Inch DIST_TO_MIDDLE_PEG = 114;//distance from alliance wall to the middle peg //TODO: find out correct distance
	const Executive auto_score_gear_middle{Chain{
		Step{Combo{
			Step{Drive_straight{(DIST_TO_MIDDLE_PEG - ROBOT_LENGTH - EXTENDED_GEAR_LENGTH) - SCORE_GEAR_APPROACH_DIST - ALIGN_DIST}},
			Step{Turn_on_light()}
		}},
		Executive{Chain{
			Step{Align()},
			Executive{Chain{
				Step{Lift_gear()},
				Executive{Chain{
					Step{Combo{
						Step{Ram{ALIGN_DIST}},
						Step{Lift_gear()},
					}},
					Executive{Chain{
						Step{Score_gear()},
						Executive{Teleop()}
					}}
				}}
			}}
		}}
	}};
	
	const Executive auto_score_gear_middle_nonvision{Chain{
		Step{Drive_straight{(DIST_TO_MIDDLE_PEG - ROBOT_LENGTH - EXTENDED_GEAR_LENGTH) - SCORE_GEAR_APPROACH_DIST}},
		Executive{Chain{
			Step{Lift_gear()},
			Executive{Chain{
				Step{Score_gear()},
				Executive{Teleop()}
			}}
		}}
	}};

	//Score a gear on the boiler-side peg
	static const Inch FIRST_DRIVE_DIST_BOILER = (115 - ROBOT_LENGTH) + .5 * ROBOT_LENGTH;//centers the robot on the turning point to align with gear peg //from testing

	auto auto_score_gear_boiler_side=[=](bool red){
		return Executive{Chain{
			Step{Drive_straight{FIRST_DRIVE_DIST_BOILER,0.02,1.0}},//NOTE: 0.02 must match that in step.cpp
			Executive{Chain{
				Step{Combo{
					Step{Turn{deg_to_rad(red?-33:33)}},//from testing
					Step{Turn_on_light()},
				}},
				Executive{Chain{
					Step{Align()},
					Executive{Chain{
						Step{Lift_gear()},
						Executive{Chain{
							Step{Combo{
								Step{Ram{6}},//drive forward a bit so score_gear can take over
								Step{Lift_gear()}
							}},
							Executive{Chain{
								Step{Score_gear()},
								Executive{Teleop()}
							}}
						}}
					}}
				}}
			}}
		}};
	};

	const Executive auto_score_gear_boiler_side_blue=auto_score_gear_boiler_side(0);
	const Executive auto_score_gear_boiler_side_red=auto_score_gear_boiler_side(1);
	
	//Crosses the baseline and continues driving to the other side of the field
	const Executive auto_baseline_extended{Chain{
		Step{Drive_straight{12*12}},
		dash
	}};
	
	//scores gear on the boilder-side of the field and then drives towards the other side of the field
	auto auto_score_gear_boiler_side_extended=[=](bool red){
		return Executive{Chain{
			Step{Drive_straight{5*12}},
			Executive{Chain{
				Step{Turn{deg_to_rad(red?-33:33)}},
				Executive{Chain{
					Step{Score_gear()},
					Executive{Chain{
						Step{Drive_straight{-2 * 12}},
						Executive{Chain{
							Step{Turn{deg_to_rad(red?33:-33)}},
							dash
						}}
					}}
				}}
			}}
		}};
	};

	const Executive auto_score_gear_boiler_side_extended_blue=auto_score_gear_boiler_side_extended(0);
	const Executive auto_score_gear_boiler_side_extended_red=auto_score_gear_boiler_side_extended(1);
	
	//scores a gear on the loading station-side peg
	static const Inch FIRST_DRIVE_DIST_LOADING = (115 - ROBOT_LENGTH) + .5 * ROBOT_LENGTH;
	auto auto_score_gear_loading_station=[=](bool red){
		return Executive{Chain{
			Step{Drive_straight{FIRST_DRIVE_DIST_LOADING,0.02,1.0}}, //TODO: set max speed to 1.0 ? or leave at default?
			Executive{Chain{
				Step{Combo{
					Step{Turn{deg_to_rad(red?33:-33)}},
					Step{Turn_on_light()},
				}},
				Executive{Chain{
					Step{Align()},
					Executive{Chain{
						Step{Lift_gear()},
						Executive{Chain{
							Step{Combo{
								Step{Ram{6}},
								Step{Lift_gear()}
							}},
							Executive{Chain{
								Step{Score_gear()},
								Executive{Teleop()}
							}}
						}}
					}}
				}}
			}}
		}};
	};
	
	const Executive auto_score_gear_loading_station_side_blue=auto_score_gear_loading_station(0);
	const Executive auto_score_gear_loading_station_side_red=auto_score_gear_loading_station(1);
	
	//Gear Loading Extended
	const auto auto_score_gear_loading_station_side_extended=[=](bool red){
		return Executive{Chain{
			Step{Drive_straight{FIRST_DRIVE_DIST_LOADING}},
			Executive{Chain{
				Step{Turn{deg_to_rad(red?31:-31)}},
				Executive{Chain{
					Step{Score_gear()},
					Executive{Chain{
						Step{Drive_straight{-12}},
						Executive{Chain{
							Step{Turn{deg_to_rad(red?-40:40)}},
							dash
						}}
					}}
				}}
			}}
		}};
	};

	const Executive auto_score_gear_loading_station_side_extended_red=auto_score_gear_loading_station_side_extended(1);
	const Executive auto_score_gear_loading_station_side_extended_blue=auto_score_gear_loading_station_side_extended(0);
	
	auto auto_score_gear_middle_extended=[=](bool right){
		return Executive{Chain{
			Step{Drive_straight{10*12}},
			Executive{Chain{
				Step{Score_gear()},
				Executive{Chain{
					Step{Drive_straight{-12}},
					Executive{Chain{
						Step{Turn{deg_to_rad(right?-45:45)}},
						Executive{Chain{
							Step{Drive_straight{3*12}},
							Executive{Chain{
								Step{Turn{deg_to_rad(right?45:-45)}},
								dash
							}}
						}}
					}}
				}}
			}}
		}};
	};

	//Gear mid Extended right (goes around the airship to the right)
	const Executive auto_score_gear_middle_extended_right=auto_score_gear_middle_extended(1);

	//Gear mid Extended left (goes around airship to the left)
	const Executive auto_score_gear_middle_extended_left=auto_score_gear_middle_extended(0);

	const Executive auto_forward{Chain{
		Step{Drive_straight{7*12}},
		Executive{Teleop()}
	}};

	const Executive motion_profile_test{Chain{
		Step{MP_drive{8*12}},
		Executive{Teleop()}
	}};

	if(!info.panel.in_use){
		//Do nothing during autonomous mode if no panel exists
		return auto_null;
	}

	switch(info.panel.auto_select){
		case 0: 
			return auto_null;
		case 1: 
			return auto_baseline;
		case 2: 
			return auto_baseline_extended;		
		case 3:
			switch(info.in.ds_info.alliance){
				case Alliance::BLUE: return auto_score_gear_boiler_side_blue;
				case Alliance::RED: return auto_score_gear_boiler_side_red;
				default: return auto_baseline;
			}
		case 4: 
			switch(info.in.ds_info.alliance){
				case Alliance::BLUE: return auto_score_gear_boiler_side_extended_blue;
				case Alliance::RED: return auto_score_gear_boiler_side_extended_red;
				default: return auto_baseline;
			}
		case 5: 
			switch(info.in.ds_info.alliance){
				case Alliance::BLUE: return auto_score_gear_loading_station_side_blue;
				case Alliance::RED: return auto_score_gear_loading_station_side_red;
				default: return auto_baseline;
			}
		case 6:
			switch(info.in.ds_info.alliance){
				case Alliance::BLUE: return auto_score_gear_loading_station_side_extended_blue;
				case Alliance::RED: return auto_score_gear_loading_station_side_extended_red; 
				default: return auto_baseline;
			}
		case 7: 
			return auto_score_gear_middle;
		case 8: 
			return auto_score_gear_middle_extended_right;
		case 9: 
			return auto_score_gear_middle_extended_left;
		case 10:
			return auto_forward;
		case 11:
			return auto_score_gear_middle_nonvision;
		case 12:
		case 13:
		case 14:
			return auto_null;
		////////////////////////////
		//
		// Tests for different steps
		//
		case 15:
			return make_test_step(Drive_straight{8*12});
		case 16:
			return make_test_step(Step{Score_gear()});
		case 17:
			return make_test_step(Turn{PI/2});
		case 18:
			return make_test_step(Align{PI/2,1});
		case 19:
			return motion_profile_test;
		default:
			return auto_null;
	}
}

Executive Autonomous::next_mode(Next_mode_info info){
	static const Time DELAY = 0.0;//seconds, TODO: base it off of the dial on the OI? 
	if(!info.autonomous) return Executive{Teleop()};
	if(info.since_switch > DELAY) return get_auto_mode(info);
	return Executive{Autonomous()};
}

Toplevel::Goal Autonomous::run(Run_info){	
	return {};//nothing, just waits
}

bool Autonomous::operator==(Autonomous const&)const{ return 1; }

#ifdef AUTONOMOUS_TEST
#include "test.h"
int main(){
	Autonomous a;
	test_executive(a);
}
#endif 
