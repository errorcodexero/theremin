#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include "../util/interface.h"
#include "../util/checked_array.h"

namespace frc{
	class Compressor;
};

class Pump_control{
	private:
	frc::Compressor *compressor;
	
	Pump_output out;
	//Talon_srx_output last_out;
	Pump_input in;
	
	unsigned since_query; //TODO: should this be time based instead or something?
	
	public:
	#define TALON_SRX_MODES X(INIT) X(PERCENT) X(SPEED) X(DISABLE)
	enum class Mode{
		#define X(NAME) NAME,
		TALON_SRX_MODES
		#undef X
	};

	private:
	Mode mode;
	
	public:
	Talon_srx_control();
	~Talon_srx_control();
	explicit Talon_srx_control(int CANBusAddress);
	
	void init(int CANBusAddress);
	void set(Talon_srx_output, bool);
	Talon_srx_input get();
	friend std::ostream& operator<<(std::ostream&,Talon_srx_control);
};

std::ostream& operator<<(std::ostream&,Talon_srx_control::Mode);

#endif
