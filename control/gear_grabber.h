#ifndef GEAR_GRABBER_H
#define GEAR_GRABBER_H

#include "../util/interface.h"
#include <set>
#include "../util/countdown_timer.h"

struct Gear_grabber{
	#define GEAR_GRABBER_GOALS(X) X(OPEN) X(CLOSE) X(X)
	enum class Goal{
		#define X(A) A,
		GEAR_GRABBER_GOALS(X)
		#undef X
	};

	struct Output{
		#define GEAR_GRABBER_PISTON_OUTPUTS(X) X(OPEN) X(CLOSE)
		enum class Piston{
			#define X(A) A,
			GEAR_GRABBER_PISTON_OUTPUTS(X)
			#undef X
		};
	
		Piston piston;
		bool gear_light;//indicator light on the OI which shows if the robot has a gear or not
		Output();
		Output(Piston,bool);
	};
	
	struct Input{
		bool has_gear;//hall-effect
		bool enabled;
		Input();
		Input(bool,bool);
	};
	
	struct Input_reader{
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};

	struct Status_detail{
		enum class State{OPEN,OPENING,CLOSING,CLOSED};
		State state;
		bool has_gear;
		Status_detail();
		Status_detail(State,bool);
	};
	
	typedef Status_detail Status;	
	
	struct Output_applicator{
		Output operator()(Robot_outputs)const;
		Robot_outputs operator()(Robot_outputs,Output)const;
	};

	struct Estimator{
		Status_detail last;	
		Countdown_timer open_timer;
		Countdown_timer close_timer;
	
		void update(Time,Input,Output);
		Status_detail get()const;
	
		Estimator();
	};
	
	Input_reader input_reader;
	Output_applicator output_applicator;
	Estimator estimator;
};


std::set<Gear_grabber::Goal> examples(Gear_grabber::Goal*);
std::set<Gear_grabber::Output> examples(Gear_grabber::Output*);
std::set<Gear_grabber::Input> examples(Gear_grabber::Input*);
std::set<Gear_grabber::Status_detail> examples(Gear_grabber::Status_detail*);

std::ostream& operator<<(std::ostream&,Gear_grabber);
std::ostream& operator<<(std::ostream&,const Gear_grabber::Input);
std::ostream& operator<<(std::ostream&,Gear_grabber::Goal);
std::ostream& operator<<(std::ostream&,Gear_grabber::Output const&);
std::ostream& operator<<(std::ostream&,const Gear_grabber::Status_detail);
std::ostream& operator<<(std::ostream&,Gear_grabber::Output::Piston);

bool operator==(const Gear_grabber::Input,const Gear_grabber::Input);
bool operator!=(const Gear_grabber::Input,const Gear_grabber::Input);
bool operator<(const Gear_grabber::Input,const Gear_grabber::Input);

bool operator<(Gear_grabber::Output const&,Gear_grabber::Output const&);
bool operator==(Gear_grabber::Output const&,Gear_grabber::Output const&);
bool operator!=(Gear_grabber::Output const&,Gear_grabber::Output const&);

bool operator<(Gear_grabber::Status_detail const&,Gear_grabber::Status_detail const&);
bool operator==(Gear_grabber::Status_detail const&,Gear_grabber::Status_detail const&);
bool operator!=(Gear_grabber::Status_detail const&,Gear_grabber::Status_detail const&);

bool operator==(const Gear_grabber::Estimator,const Gear_grabber::Estimator);
bool operator!=(const Gear_grabber::Estimator,const Gear_grabber::Estimator);

Gear_grabber::Output control(Gear_grabber::Status,Gear_grabber::Goal);
Gear_grabber::Status status(Gear_grabber::Status_detail);
bool ready(Gear_grabber::Status,Gear_grabber::Goal);

#endif
