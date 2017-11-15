#ifndef	GRABBER_ARM_H
#define GRABBER_ARM_H

#include <set>
#include "../util/interface.h"
#include "../util/countdown_timer.h"

struct Grabber_arm{
	#define GRABBER_ARM_GOALS(F) F(DOWN) F(UP) F(X)
	enum class Goal{
		#define X(A) A,
		GRABBER_ARM_GOALS(X)
		#undef X
	};
	
	#define GRABBER_ARM_OUTPUTS(X) X(DOWN) X(UP)
	enum class Output{
		#define X(A) A,
		GRABBER_ARM_OUTPUTS(X)
		#undef X
	};

	struct Input{
		bool enabled, limit_switch;
		Input();
		Input(bool,bool);
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

std::set<Grabber_arm::Goal> examples(Grabber_arm::Goal*);
std::set<Grabber_arm::Input> examples(Grabber_arm::Input*);
std::set<Grabber_arm::Output> examples(Grabber_arm::Output*);
std::set<Grabber_arm::Status_detail> examples(Grabber_arm::Status_detail*);

std::ostream& operator<<(std::ostream&,Grabber_arm::Goal);
std::ostream& operator<<(std::ostream&,Grabber_arm::Input);
std::ostream& operator<<(std::ostream&,Grabber_arm::Output);
std::ostream& operator<<(std::ostream&,Grabber_arm::Status_detail);
std::ostream& operator<<(std::ostream&,Grabber_arm const&);

bool operator<(Grabber_arm::Input,Grabber_arm::Input);
bool operator==(Grabber_arm::Input,Grabber_arm::Input);
bool operator!=(Grabber_arm::Input,Grabber_arm::Input);

bool operator==(Grabber_arm::Estimator,Grabber_arm::Estimator);
bool operator!=(Grabber_arm::Estimator,Grabber_arm::Estimator);

bool operator==(Grabber_arm,Grabber_arm);
bool operator!=(Grabber_arm,Grabber_arm);

Grabber_arm::Output control(Grabber_arm::Status,Grabber_arm::Goal);
Grabber_arm::Status status(Grabber_arm::Status);
bool ready(Grabber_arm::Status,Grabber_arm::Goal);

#endif
