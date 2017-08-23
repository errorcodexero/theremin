#ifndef	ROLLER_ARM_H
#define ROLLER_ARM_H

#include <set>
#include "../util/interface.h"
#include "../util/countdown_timer.h"

struct Roller_arm{
	#define ROLLER_ARM_GOALS(F) F(STOW) F(LOW) F(X)
	enum class Goal{
		#define X(A) A,
		ROLLER_ARM_GOALS(X)
		#undef X
	};
	
	#define ROLLER_ARM_OUTPUTS(X) X(STOW) X(LOW)
	enum class Output{
		#define X(A) A,
		ROLLER_ARM_OUTPUTS(X)
		#undef X
	};

	struct Input{
		bool enabled;
		Input();
		Input(bool);
	};

	enum class Status_detail{STOW,STOWING,LOWERING,LOW};
	
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

std::set<Roller_arm::Goal> examples(Roller_arm::Goal*);
std::set<Roller_arm::Input> examples(Roller_arm::Input*);
std::set<Roller_arm::Output> examples(Roller_arm::Output*);
std::set<Roller_arm::Status_detail> examples(Roller_arm::Status_detail*);

std::ostream& operator<<(std::ostream&,Roller_arm::Goal);
std::ostream& operator<<(std::ostream&,Roller_arm::Input);
std::ostream& operator<<(std::ostream&,Roller_arm::Output);
std::ostream& operator<<(std::ostream&,Roller_arm::Status_detail);
std::ostream& operator<<(std::ostream&,Roller_arm const&);

bool operator<(Roller_arm::Input,Roller_arm::Input);
bool operator==(Roller_arm::Input,Roller_arm::Input);
bool operator!=(Roller_arm::Input,Roller_arm::Input);

bool operator==(Roller_arm::Estimator,Roller_arm::Estimator);
bool operator!=(Roller_arm::Estimator,Roller_arm::Estimator);

bool operator==(Roller_arm,Roller_arm);
bool operator!=(Roller_arm,Roller_arm);

Roller_arm::Output control(Roller_arm::Status,Roller_arm::Goal);
Roller_arm::Status status(Roller_arm::Status);
bool ready(Roller_arm::Status,Roller_arm::Goal);

#endif
