#include "navx_control.h"
#include "AHRS.h"

using namespace std;

Navx_control::Navx_control():ahrs(NULL),in(){}
Navx_control::Navx_control(frc::SerialPort::Port port):ahrs(NULL),in(){
	init(port);
}

Navx_control::~Navx_control(){
	delete ahrs;
}

void Navx_control::init(frc::SerialPort::Port port){
	assert(!ahrs);	
	ahrs = new AHRS(port);
	assert(ahrs);
	ahrs->ZeroYaw();
}

Navx_input Navx_control::get(){
	#define X(A,B,C) in.B = ahrs->C();
	NAVX_ITEMS(X)
	#undef X

	return in;
}

ostream& operator<<(ostream& o,Navx_control){
	o<<"Navx_control()";
	return o;
}


