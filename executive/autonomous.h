#ifndef AUTONOMOUS_H
#define AUTONOMOUS_H

#include "executive.h"

struct Autonomous: public Executive_impl<Autonomous>{
	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Autonomous const&)const;
};

#endif
