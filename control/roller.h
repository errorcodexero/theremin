#ifndef ROLLER_H
#define ROLLER_H

#include <iostream>
#include <set>
#include "../util/interface.h"
#include "nop.h"

struct Roller{
	enum class Goal{IN,OFF,OUT};
	
	using Input=Nop::Input;
	
	typedef Input Status_detail;

	typedef Status_detail Status;

	using Input_reader=Nop::Input_reader;

	typedef Goal Output;
	
	struct Output_applicator{
		Robot_outputs operator()(Robot_outputs,Roller::Output)const;
		Roller::Output operator()(Robot_outputs)const;	
	};

	using Estimator=Nop::Estimator;

	Input_reader input_reader;
	Estimator estimator;
	Output_applicator output_applicator; 
};

std::ostream& operator<<(std::ostream&,Roller::Goal);
std::ostream& operator<<(std::ostream&,Roller);

bool operator<(Roller::Status_detail,Roller::Status_detail);
bool operator==(Roller::Status_detail,Roller::Status_detail);
bool operator!=(Roller::Status_detail,Roller::Status_detail);

bool operator==(Roller::Output_applicator,Roller::Output_applicator);

bool operator==(Roller,Roller);
bool operator!=(Roller,Roller);

std::set<Roller::Goal> examples(Roller::Goal*);
std::set<Roller::Status_detail> examples(Roller::Status_detail*);

Roller::Output control(Roller::Status_detail, Roller::Goal);
Roller::Status status(Roller::Status_detail);
bool ready(Roller::Status, Roller::Goal);

#endif
