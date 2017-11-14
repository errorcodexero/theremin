#ifndef	PINCHERS_H
#define PINCHERS_H

#include <set>
#include "../util/interface.h"
#include "../util/countdown_timer.h"

struct Pinchers{
	#define PINCHERS_GOALS(F) F(OPEN) F(CLOSE) F(X)
	enum class Goal{
		#define X(A) A,
		PINCHERS_GOALS(X)
		#undef X
	};
	
	#define PINCHERS_OUTPUTS(X) X(OPEN) X(CLOSE)
	enum class Output{
		#define X(A) A,
		PINCHERS_OUTPUTS(X)
		#undef X
	};

	struct Input{
		bool enabled;
		Input();
		Input(bool);
	};

	enum class Status_detail{OPEN,OPENING,CLOSING,CLOSED};
	
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

std::set<Pinchers::Goal> examples(Pinchers::Goal*);
std::set<Pinchers::Input> examples(Pinchers::Input*);
std::set<Pinchers::Output> examples(Pinchers::Output*);
std::set<Pinchers::Status_detail> examples(Pinchers::Status_detail*);

std::ostream& operator<<(std::ostream&,Pinchers::Goal);
std::ostream& operator<<(std::ostream&,Pinchers::Input);
std::ostream& operator<<(std::ostream&,Pinchers::Output);
std::ostream& operator<<(std::ostream&,Pinchers::Status_detail);
std::ostream& operator<<(std::ostream&,Pinchers const&);

bool operator<(Pinchers::Input,Pinchers::Input);
bool operator==(Pinchers::Input,Pinchers::Input);
bool operator!=(Pinchers::Input,Pinchers::Input);

bool operator==(Pinchers::Estimator,Pinchers::Estimator);
bool operator!=(Pinchers::Estimator,Pinchers::Estimator);

bool operator==(Pinchers,Pinchers);
bool operator!=(Pinchers,Pinchers);

Pinchers::Output control(Pinchers::Status,Pinchers::Goal);
Pinchers::Status status(Pinchers::Status);
bool ready(Pinchers::Status,Pinchers::Goal);

#endif
