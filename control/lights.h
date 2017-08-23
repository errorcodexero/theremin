#ifndef LIGHTS_H
#define LIGHTS_H

#include <iostream>
#include <set>
#include "../util/interface.h"
#include "nop.h"

struct Lights{
	enum class Loading_indicator{BALLS,GEARS};
	enum class Blinky_mode{FLASH, NO_FLASH};
	struct Goal{
		Loading_indicator loading_indicator;
		bool camera_light;
		Blinky_mode blinky_mode;

		Goal();
		Goal(Loading_indicator,bool,Blinky_mode);
	};
	
	using Input=Nop::Input;

	typedef Time Status_detail;

	typedef Status_detail Status;

	using Input_reader=Nop::Input_reader;

	struct Output{
		bool loading_indicator,camera_light;
		double blinky_number;

		Output();
		Output(bool,bool,double);
	};
	
	struct Output_applicator{
		Output operator()(Robot_outputs)const;	
		Robot_outputs operator()(Robot_outputs,Output)const;
	};

	struct Estimator{
		Status_detail time;
		void update(Time,Input,Output);
		Status_detail get()const;

		Estimator();
	};

	Input_reader input_reader;
	Estimator estimator;
	Output_applicator output_applicator; 
};

std::ostream& operator<<(std::ostream&,Lights::Loading_indicator);
std::ostream& operator<<(std::ostream&,Lights::Goal);
std::ostream& operator<<(std::ostream&,Lights::Output);
std::ostream& operator<<(std::ostream&,Lights);

bool operator<(Lights::Goal,Lights::Goal);
bool operator==(Lights::Goal,Lights::Goal);

bool operator<(Lights::Output,Lights::Output);
bool operator==(Lights::Output,Lights::Output);
bool operator!=(Lights::Output,Lights::Output);

bool operator==(Lights::Output_applicator,Lights::Output_applicator);

bool operator==(Lights::Estimator,Lights::Estimator);
bool operator!=(Lights::Estimator,Lights::Estimator);

bool operator==(Lights,Lights);
bool operator!=(Lights,Lights);

std::set<Lights::Goal> examples(Lights::Goal*);
std::set<Lights::Status_detail> examples(Lights::Status_detail*);
std::set<Lights::Output> examples(Lights::Output*);

Lights::Output control(Lights::Status_detail, Lights::Goal);
Lights::Status status(Lights::Status_detail);
bool ready(Lights::Status, Lights::Goal);

#endif
