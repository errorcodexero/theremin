#ifndef INTERFACE_H
#define INTERFACE_H

#include <iosfwd>
#include <bitset>
#include "driver_station_interface.h"
#include "maybe_inline.h"
#include "checked_array.h"
#include "../input/pixycam/PixyUART.h"

typedef double Time;//Seconds
typedef bool Solenoid_output;

using Pwm_output = double;

enum class Relay_output{_00,_01,_10,_11};
std::ostream& operator<<(std::ostream&,Relay_output);

class Digital_out{
	public:
	enum class Type{INPUT,_1,_0,ENCODER};

	private:
	Type type_;
	int encoder_index_;
	bool input_a_;

	public:
	Digital_out();

	Type type()const;
	int encoder_index()const;
	bool input_a()const;

	static Digital_out input();
	static Digital_out one();
	static Digital_out zero();
	static Digital_out encoder(int encoder_index,bool input_a);
};

struct PID_values{
	float p,i,d,f;
	PID_values();
};
std::ostream& operator<<(std::ostream&,PID_values const&);
bool operator==(PID_values const&,PID_values const&);
bool operator<(PID_values const&,PID_values const&);

struct Talon_srx_input{
	int encoder_position;
	bool fwd_limit_switch;
	bool rev_limit_switch;
	bool a;
	bool b;
	int velocity;
	double current;
	Talon_srx_input():encoder_position(0),fwd_limit_switch(0),rev_limit_switch(0),a(0),b(0),velocity(0),current(0){}
};


struct Talon_srx_output{
	PID_values pid;
	double power_level;
	double speed;
	enum class Mode{PERCENT,SPEED};//percent means percent voltage on the in terminals on the talon
	Mode mode;

	Talon_srx_output():power_level(0),speed(0),mode(Talon_srx_output::Mode::PERCENT){}

	static Talon_srx_output percent(double);
	static Talon_srx_output closed_loop(double);
};

std::ostream& operator<<(std::ostream&,Talon_srx_output::Mode);
std::ostream& operator<<(std::ostream&,Talon_srx_output);
bool operator==(Talon_srx_output,Talon_srx_output);
bool operator!=(Talon_srx_output,Talon_srx_output);
bool operator<(Talon_srx_output,Talon_srx_output);

std::ostream& operator<<(std::ostream&,Talon_srx_input);
bool operator==(Talon_srx_input,Talon_srx_input);
bool operator!=(Talon_srx_input,Talon_srx_input);
bool operator<(Talon_srx_input,Talon_srx_input);

std::ostream& operator<<(std::ostream&,Digital_out);
bool operator<(Digital_out,Digital_out);
bool operator==(Digital_out,Digital_out);
bool operator!=(Digital_out,Digital_out);

static const unsigned TALON_SRXS = 1;

struct Robot_outputs{
	static const unsigned PWMS=10;//Number of ports on the digital sidecar; there can be up to 20 using the MXP on the roboRIO which we don't do
	Checked_array<Pwm_output,PWMS> pwm;
	
	static const unsigned SOLENOIDS=8;
	Checked_array<Solenoid_output,SOLENOIDS> solenoid;
	
	static const unsigned RELAYS=4;
	Checked_array<Relay_output,RELAYS> relay;
	
	static const unsigned DIGITAL_IOS=11;//there are actually 26 on the roboRIO if you count the MXP, but that varies depending on whether they're set as dios or pwm
	Checked_array<Digital_out,DIGITAL_IOS> digital_io;
	
	static const unsigned TALON_SRX_OUTPUTS=TALON_SRXS;//FIXME: talon initializaitons
	Checked_array<Talon_srx_output, TALON_SRX_OUTPUTS> talon_srx;
	
	//could add in some setup for the analog inputs
	
	static const unsigned DRIVER_STATION_DIGITAL_OUTPUTS = Driver_station_output::DIGITAL_OUTPUTS;
	Driver_station_output driver_station;
	bool pump_auto;

	Robot_outputs();
};

std::ostream& operator<<(std::ostream& o, Talon_srx_output);

std::ostream& operator<<(std::ostream& o, Talon_srx_input);

bool operator<(Robot_outputs,Robot_outputs);
bool operator==(Robot_outputs,Robot_outputs);
bool operator!=(Robot_outputs,Robot_outputs);
std::ostream& operator<<(std::ostream& o,Robot_outputs);

//limitation of FRC coms//TODO look into this
#define JOY_AXES 8
#define JOY_BUTTONS 13

struct Joystick_data{
	Checked_array<double,JOY_AXES> axis;
	std::bitset<JOY_BUTTONS> button;
	int pov;
	
	Joystick_data();

	static Maybe<Joystick_data> parse(std::string const&);
};
bool operator<(Joystick_data,Joystick_data);
bool operator==(Joystick_data,Joystick_data);
bool operator!=(Joystick_data,Joystick_data);
std::ostream& operator<<(std::ostream&,Joystick_data);

//We may need to add support for other modes at some point.
struct Robot_mode{
	bool autonomous;
	bool enabled;
	
	Robot_mode();
};
bool operator<(Robot_mode,Robot_mode);
bool operator==(Robot_mode,Robot_mode);
bool operator!=(Robot_mode,Robot_mode);
std::ostream& operator<<(std::ostream&,Robot_mode);

enum class Alliance{RED,BLUE,INVALID};
struct DS_info{
	bool connected;
	Alliance alliance;
	int location;
	DS_info();
};
bool operator<(DS_info const&,DS_info const&);
bool operator==(DS_info const&,DS_info const&);
bool operator!=(DS_info const&,DS_info const&);
std::ostream& operator<<(std::ostream&,DS_info const&);

enum class Digital_in{OUTPUT,_0,_1,ENCODER};
std::ostream& operator<<(std::ostream&,Digital_in);
std::set<Digital_in> examples(Digital_in*);

typedef int Encoder_output;//TODO: is this not an input?

struct Digital_inputs{
	Checked_array<Digital_in,Robot_outputs::DIGITAL_IOS> in;
	static const unsigned ENCODERS=Robot_outputs::DIGITAL_IOS/2;
	Checked_array<Maybe_inline<Encoder_output>,ENCODERS> encoder;

	Digital_inputs();
};
bool operator<(Digital_inputs const&,Digital_inputs const&);
bool operator==(Digital_inputs const&,Digital_inputs const&);
bool operator!=(Digital_inputs const&,Digital_inputs const&);
std::ostream& operator<<(std::ostream&,Digital_inputs const&);

struct Camera{
	static const double FOV; //degrees
	bool enabled;
	std::vector<Pixy::Block> blocks;
	
	Camera();
};
bool operator<(Camera const&,Camera const&);
bool operator==(Camera const&,Camera const&);
bool operator!=(Camera const&,Camera const&);
std::ostream& operator<<(std::ostream&,Camera const&);

typedef float Volt;
typedef double Rad; //radians, clockwise

struct Robot_inputs{
	Robot_mode robot_mode;
	Time now;//time since robot code started running.

	DS_info ds_info;

	static const unsigned JOYSTICKS=3; //ports are 0-5, so there are actually six, but we only use the first three so we'll only consider them to limit print outs
	Checked_array<Joystick_data,JOYSTICKS> joystick;

	Digital_inputs digital_io;	

	static const unsigned ANALOG_INPUTS=4;
	Checked_array<Volt,ANALOG_INPUTS> analog;

	static const unsigned TALON_SRX_INPUTS=TALON_SRXS;
	Checked_array<Talon_srx_input, TALON_SRX_INPUTS> talon_srx;
	
	Driver_station_input driver_station;
	Rad orientation;
		
	static const unsigned CURRENT=16;
	Checked_array<double,CURRENT> current;
	bool pump;

	Camera camera;

	Robot_inputs();
};
bool operator<(Robot_inputs,Robot_inputs);
bool operator==(Robot_inputs,Robot_inputs);
bool operator!=(Robot_inputs,Robot_inputs);
std::ostream& operator<<(std::ostream& o,Robot_inputs);
Robot_inputs rand(Robot_inputs*);
Robot_inputs random_inputs();

#endif
