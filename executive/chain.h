#ifndef CHAIN_H
#define CHAIN_H

#include "executive.h"
#include "step.h"

struct Chain:Executive_impl<Chain>{
	//enum class Failure_procedure{TREAT_AS_SUCCESS,ENTER_TELEOP,ABORT};
	Step step;
	Executive next;

	Chain(Step,Executive);

	Toplevel::Goal run(Run_info);
	Executive next_mode(Next_mode_info);
	bool operator==(Chain const&)const;
	std::unique_ptr<Executive_interface> clone()const;
	void display(std::ostream&)const;
};

#endif
