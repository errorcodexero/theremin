#ifndef SHOOTER_H
#define SHOOTER_H

#include <iostream>
#include <set>
#include "../util/interface.h"
#include "../util/util.h"
#include "../util/countdown_timer.h"
#include "../util/quick.h"
#include "nop.h"

struct Shooter{
	enum class Goal{FORWARD,OFF,REVERSE};
	
	using Status_detail = Nop::Status_detail;
	
	using Status = Status_detail;

	using Input = Nop::Input;

	using Input_reader = Nop::Input_reader;

	using Output = Goal;
	
	struct Output_applicator{
		Shooter::Output operator()(Robot_outputs const&)const;
		Robot_outputs operator()(Robot_outputs,Output)const;
	};

	using Estimator = Nop::Estimator;

	Input_reader input_reader;
	Estimator estimator;
	Output_applicator output_applicator;
};

std::ostream& operator<<(std::ostream&,Shooter::Goal);
std::ostream& operator<<(std::ostream&,Shooter);

bool operator==(Shooter::Output_applicator,Shooter::Output_applicator);

bool operator==(Shooter,Shooter);
bool operator!=(Shooter,Shooter);

std::set<Shooter::Goal> examples(Shooter::Goal*);

Shooter::Status status(Shooter::Status_detail);
Shooter::Output control(Shooter::Status_detail,Shooter::Goal);
bool ready(Shooter::Status,Shooter::Goal);

#endif

