#ifndef DOZER_H
#define DOZER_H

#include <iostream>
#include <set>
#include "../util/countdown_timer.h"
#include "../util/interface.h"
#include "../util/util.h"
#include "../util/driver_station_interface.h"

struct Dozer{ //door-opener that releases the dozer from stowed position
	enum class Goal{IN,STOP,OUT};
	
	enum class Status_detail{IN,OUT,PROBABLY_OUT,PROBABLY_IN,UNKNOWN};
	
	typedef Status_detail Status;

	struct Input{
		bool enabled;
	};
	
	struct Input_reader{
		Dozer::Input operator()(Robot_inputs const&)const;
		Robot_inputs operator()(Robot_inputs,Dozer::Input)const;
	};

	typedef Goal Output;
	
	struct Output_applicator{
		Dozer::Output operator()(Robot_outputs)const;
		Robot_outputs operator()(Robot_outputs,Dozer::Output)const;
	};

	struct Estimator{
		Status_detail last;
		Countdown_timer state_timer;
		Countdown_timer refresh_timer;

		Dozer::Output last_output;
		Dozer::Status_detail get()const;
		void update(Time,Dozer::Input,Dozer::Output);
		Estimator();
	};

	Input_reader input_reader;
	Estimator estimator;
	Output_applicator output_applicator;
};

std::ostream& operator<<(std::ostream&,Dozer::Goal);
std::ostream& operator<<(std::ostream&,Dozer::Input);
std::ostream& operator<<(std::ostream&,Dozer::Status_detail);
std::ostream& operator<<(std::ostream&,Dozer::Estimator);
std::ostream& operator<<(std::ostream&,Dozer);

bool operator==(Dozer::Input,Dozer::Input);
bool operator!=(Dozer::Input,Dozer::Input);
bool operator<(Dozer::Input,Dozer::Input);

bool operator==(Dozer::Input_reader,Dozer::Input_reader);
bool operator<(Dozer::Input_reader,Dozer::Input_reader);

bool operator==(Dozer::Estimator,Dozer::Estimator);
bool operator!=(Dozer::Estimator,Dozer::Estimator);

bool operator==(Dozer::Output_applicator,Dozer::Output_applicator);

bool operator==(Dozer,Dozer);
bool operator!=(Dozer,Dozer);

std::set<Dozer::Input> examples(Dozer::Input*);
std::set<Dozer::Goal> examples(Dozer::Goal*);
std::set<Dozer::Status_detail> examples(Dozer::Status_detail*);

Dozer::Output control(Dozer::Status_detail,Dozer::Goal);
Dozer::Status status(Dozer::Status_detail);
bool ready(Dozer::Status,Dozer::Goal);

#endif
