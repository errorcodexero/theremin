#ifndef	CLIMBER_H
#define CLIMBER_H

#include <set>
#include "../util/interface.h"
#include "nop.h"
#include "../util/countdown_timer.h"

struct Climber{
	enum class Goal{STANDARD_CLIMB,TURBO_CLIMB,STOP,RELEASE};
	using Output = Goal;

	using Input = Nop::Input;

	using Status = Nop::Status;
	
	using Status_detail = Status;
	
	using Input_reader = Nop::Input_reader;

	struct Output_applicator{
		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs const&)const;
	};

	using Estimator = Nop::Estimator;

	Input_reader input_reader;
	Output_applicator output_applicator;
	Estimator estimator;
};

std::set<Climber::Goal> examples(Climber::Goal*);

std::ostream& operator<<(std::ostream&,Climber::Goal);
std::ostream& operator<<(std::ostream&,Climber const&);

bool operator==(Climber,Climber);
bool operator!=(Climber,Climber);

Climber::Output control(Climber::Status,Climber::Goal);
bool ready(Climber::Status,Climber::Goal);
Climber::Status status(Climber::Status_detail);

#endif
