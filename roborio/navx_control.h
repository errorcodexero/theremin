#ifndef NAVX_CONTROL_H
#define NAVX_CONTROL_H

#include "../util/interface.h"

class AHRS;

class Navx_control{
	private:
	AHRS* ahrs;
	Navx_input in;
	
	public:
	Navx_control();
	~Navx_control();

	Navx_input get();
		
	void init();
	friend std::ostream& operator<<(std::ostream&,Navx_control);
};

#endif
