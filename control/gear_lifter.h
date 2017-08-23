#ifndef	GEAR_LIFTER_H
#define GEAR_LIFTER_H

#include "../util/interface.h"
#include <set>
#include "../util/countdown_timer.h"

struct Gear_lifter{
	#define GEAR_LIFTER_GOALS(X) X(DOWN) X(UP) X(X)
	enum class Goal{
		#define X(NAME) NAME,
		GEAR_LIFTER_GOALS(X)
		#undef X
	};

	#define GEAR_LIFTER_OUTPUTS(X) X(DOWN) X(UP)
	enum class Output{
		#define X(NAME) NAME,
		GEAR_LIFTER_OUTPUTS(X)
		#undef X
	};
	
	struct Input{
		bool enabled, limit_switch;
		Input();
		Input(bool, bool);
	};

	enum class Status_detail{DOWN,GOING_DOWN,GOING_UP,UP};
	
	typedef Status_detail Status;
	
	struct Input_reader{
		Input operator()(Robot_inputs const&)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};

	struct Output_applicator{
		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs const&)const;
	};

	struct Estimator{
		Status_detail last;
		Countdown_timer state_timer;		

		void update(Time,Input,Output);
		Status_detail get()const;
		Estimator();
	};

	Input_reader input_reader;
	Output_applicator output_applicator;
	Estimator estimator;
};

std::set<Gear_lifter::Goal> examples(Gear_lifter::Goal*);
std::set<Gear_lifter::Input> examples(Gear_lifter::Input*);
std::set<Gear_lifter::Output> examples(Gear_lifter::Output*);
std::set<Gear_lifter::Status_detail> examples(Gear_lifter::Status_detail*);

std::ostream& operator<<(std::ostream&,Gear_lifter::Goal);
std::ostream& operator<<(std::ostream&,Gear_lifter::Input);
std::ostream& operator<<(std::ostream&,Gear_lifter::Output);
std::ostream& operator<<(std::ostream&,Gear_lifter::Status_detail);
std::ostream& operator<<(std::ostream&,Gear_lifter const&);

bool operator<(Gear_lifter::Input,Gear_lifter::Input);
bool operator==(Gear_lifter::Input,Gear_lifter::Input);
bool operator!=(Gear_lifter::Input,Gear_lifter::Input);

bool operator==(Gear_lifter::Estimator,Gear_lifter::Estimator);
bool operator!=(Gear_lifter::Estimator,Gear_lifter::Estimator);

bool operator==(Gear_lifter,Gear_lifter);
bool operator!=(Gear_lifter,Gear_lifter);

Gear_lifter::Output control(Gear_lifter::Status,Gear_lifter::Goal);
Gear_lifter::Status status(Gear_lifter::Status);
bool ready(Gear_lifter::Status,Gear_lifter::Goal);

#endif
