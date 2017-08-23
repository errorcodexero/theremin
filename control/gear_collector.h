#ifndef GEAR_COLLECTOR_H
#define GEAR_COLLECTOR_H

#include "gear_grabber.h"
#include "gear_lifter.h"
#include "roller.h"
#include "roller_arm.h"

struct Gear_collector{
	#define GEAR_COLLECTOR_ITEMS(X) X(Gear_grabber,gear_grabber) X(Gear_lifter,gear_lifter) X(Roller,roller) X(Roller_arm,roller_arm)

	struct Input{
		#define X(A,B) A::Input B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X
	};

	struct Output{
		#define X(A,B) A::Output B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X
	};

	struct Goal{
		#define X(A,B) A::Goal B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X
		bool manual_override;
		Goal();
		Goal(Gear_grabber::Goal,Gear_lifter::Goal,Roller::Goal,Roller_arm::Goal);
		
	};

	struct Input_reader{
		#define X(A,B) A::Input_reader B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X

		Input operator()(Robot_inputs const&)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};

	struct Output_applicator{
		#define X(A,B) A::Output_applicator B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X

		Output operator()(Robot_outputs const&)const;
		Robot_outputs operator()(Robot_outputs,Output const&)const;
	};

	struct Status{
		#define X(A,B) A::Status B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X
	};

	struct Status_detail{
		#define X(A,B) A::Status_detail B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X
	};

	struct Estimator{
		#define X(A,B) A::Estimator B;
		GEAR_COLLECTOR_ITEMS(X)
		#undef X

		void update(Time,Input,Output);
		Status_detail get()const;
	};

	Estimator estimator;
	Input_reader input_reader;
	Output_applicator output_applicator;
};

bool operator!=(Gear_collector const&,Gear_collector const&);
bool operator==(Gear_collector::Input,Gear_collector::Input);
bool operator!=(Gear_collector::Input,Gear_collector::Input);
bool operator<(Gear_collector::Input,Gear_collector::Input);
bool operator<(Gear_collector::Output,Gear_collector::Output);
bool operator==(Gear_collector::Output,Gear_collector::Output);
bool operator!=(Gear_collector::Output,Gear_collector::Output);
bool operator<(Gear_collector::Goal,Gear_collector::Goal);
bool operator<(Gear_collector::Status,Gear_collector::Status);
bool operator==(Gear_collector::Status,Gear_collector::Status);
bool operator!=(Gear_collector::Status,Gear_collector::Status);
bool operator<(Gear_collector::Status_detail,Gear_collector::Status_detail);
bool operator==(Gear_collector::Status_detail,Gear_collector::Status_detail);
bool operator!=(Gear_collector::Status_detail,Gear_collector::Status_detail);
bool operator!=(Gear_collector::Estimator,Gear_collector::Estimator);

std::ostream& operator<<(std::ostream&,Gear_collector::Input const&);
std::ostream& operator<<(std::ostream&,Gear_collector::Output const&);
std::ostream& operator<<(std::ostream&,Gear_collector::Status const&);
std::ostream& operator<<(std::ostream&,Gear_collector::Status_detail const&);
std::ostream& operator<<(std::ostream&,Gear_collector::Goal const&);
std::ostream& operator<<(std::ostream&,Gear_collector const&);

Gear_collector::Status status(Gear_collector::Status_detail const&);
Gear_collector::Output control(Gear_collector::Status_detail const&,Gear_collector::Goal const&);
bool ready(Gear_collector::Status const&,Gear_collector::Goal const&);

std::set<Gear_collector::Input> examples(Gear_collector::Input*);
std::set<Gear_collector::Status_detail> examples(Gear_collector::Status_detail*);

#endif
