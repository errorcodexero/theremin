#include "drivebase.h"
#include <iostream>
#include <math.h>
#include "../util/util.h"
//temp
#include "../util/point.h"
//end temp
using namespace std;

//these are all off by one
#define L_MOTOR_LOC_1 0
#define L_MOTOR_LOC_2 1
#define R_MOTOR_LOC_1 2
#define R_MOTOR_LOC_2 3

unsigned pdb_location(Drivebase::Motor m){
	#define X(NAME,INDEX) if(m==Drivebase::NAME) return INDEX;
	//WILL NEED CORRECT VALUES
	X(LEFT1,0)
	X(LEFT2,1)
	X(RIGHT1,2)
	X(RIGHT2,13)
	#undef X
	assert(0);
	//assert(m>=0 && m<Drivebase::MOTORS);
}

int encoderconv(Maybe_inline<Encoder_output> encoder){
	if(encoder) return *encoder;
	return 10000;
}

const unsigned int TICKS_PER_REVOLUTION=200;
const double WHEEL_DIAMETER=6.0;
const double WHEEL_CIRCUMFERENCE=WHEEL_DIAMETER*PI;//inches
const double INCHES_PER_TICK=WHEEL_CIRCUMFERENCE/(double)TICKS_PER_REVOLUTION;

double ticks_to_inches(const int ticks){
	return ticks*INCHES_PER_TICK;
}

Drivebase::Distances ticks_to_inches(const Drivebase::Encoder_ticks ticks){
	Drivebase::Distances d = {0.0,0.0};
	#define X(TYPE,SIDE) d.SIDE = ticks_to_inches(ticks.SIDE);
	DISTANCES_ITEMS(X)
	#undef X
	return d;
}

int inches_to_ticks(const double inches){
	return (int)(inches/(INCHES_PER_TICK));
}

#define L_ENCODER_PORTS 0,1
#define R_ENCODER_PORTS 2,3
#define L_ENCODER_LOC 0
#define R_ENCODER_LOC 1

Robot_inputs Drivebase::Input_reader::operator()(Robot_inputs all,Input in)const{
	for(unsigned i=0;i<MOTORS;i++){
		all.current[pdb_location((Motor)i)]=in.current[i];
	}
	auto set=[&](unsigned index,Digital_in value){
		all.digital_io.in[index]=value;
	};
	auto encoder=[&](unsigned a,unsigned b,Encoder_info e){
		set(a,e.first);
		set(b,e.second);
	};
	encoder(L_ENCODER_PORTS,in.left);
	encoder(R_ENCODER_PORTS,in.right);
	all.digital_io.encoder[L_ENCODER_LOC] = -inches_to_ticks(in.distances.l);
	all.digital_io.encoder[R_ENCODER_LOC] = inches_to_ticks(in.distances.r);
	return all;
}

Drivebase::Input Drivebase::Input_reader::operator()(Robot_inputs const& in)const{
	auto encoder_info=[&](unsigned a, unsigned b){
		return make_pair(in.digital_io.in[a],in.digital_io.in[b]);
	};
	return Drivebase::Input{
		[&](){
			array<double,Drivebase::MOTORS> r;
			for(unsigned i=0;i<Drivebase::MOTORS;i++){
				Drivebase::Motor m=(Drivebase::Motor)i;
				r[i]=in.current[pdb_location(m)];
			}
			return r;
		}(),
		encoder_info(L_ENCODER_PORTS),
		encoder_info(R_ENCODER_PORTS),
		{
			-ticks_to_inches(encoderconv(in.digital_io.encoder[L_ENCODER_LOC])),
			ticks_to_inches(encoderconv(in.digital_io.encoder[R_ENCODER_LOC]))
		}
	};
}

float range(const Robot_inputs in){
	float volts=in.analog[2];
	const float voltsperinch=1; 
	float inches=volts*voltsperinch;
	return inches;
}

Drivebase::Encoder_ticks operator+(Drivebase::Encoder_ticks const& a,Drivebase::Encoder_ticks const& b){
	Drivebase::Encoder_ticks sum = {
		#define X(TYPE,SIDE) 0,
		ENCODER_TICKS(X)
		#undef X
	};
	#define X(TYPE,SIDE) sum.SIDE = a.SIDE + b.SIDE;
	ENCODER_TICKS(X)
	#undef X
	return sum;
}


Drivebase::Distances operator+(Drivebase::Distances const& a,Drivebase::Distances const& b){
	Drivebase::Distances sum = {
		#define X(TYPE,SIDE) 0,
		DISTANCES_ITEMS(X)
		#undef X
	};
	#define X(TYPE,SIDE) sum.SIDE = a.SIDE + b.SIDE;
	DISTANCES_ITEMS(X)
	#undef X
	return sum;
}

Drivebase::Distances& operator+=(Drivebase::Distances& a,Drivebase::Distances const& b){
	#define X(TYPE,SIDE) a.SIDE += b.SIDE;
	DISTANCES_ITEMS(X)
	#undef X
	return a;
}

Drivebase::Encoder_ticks operator-(Drivebase::Encoder_ticks const& a){
	Drivebase::Encoder_ticks opposite = {
		#define X(TYPE,SIDE) -a.SIDE,
		ENCODER_TICKS(X)
		#undef X
	};
	return opposite;
}

Drivebase::Encoder_ticks operator-(Drivebase::Encoder_ticks const& a,Drivebase::Encoder_ticks const& b){
	return a + (-b);
}

bool operator==(Drivebase::Distances const& a,Drivebase::Distances const& b){
	#define X(TYPE,SIDE) if(a.SIDE != b.SIDE) return false;
	DISTANCES_ITEMS(X)
	#undef X
	return true;
}

bool operator!=(Drivebase::Distances const& a,Drivebase::Distances const& b){
	return !(a==b);
}

ostream& operator<<(ostream& o,Drivebase::Distances const& a){
	o<<"Distances(";
	#define X(TYPE,SIDE) o<</*""#SIDE<<":"<<*/a.SIDE<<" ";
	DISTANCES_ITEMS(X)
	#undef X
	return o<<")";
}

bool operator<(Drivebase::Distances const& a,Drivebase::Distances const& b){
	#define X(TYPE,SIDE) if(a.SIDE >= b.SIDE) return false;
	DISTANCES_ITEMS(X)
	#undef X
	return true;
}

Drivebase::Distances fabs(Drivebase::Distances const& a){
	Drivebase::Distances pos = {
		#define X(TYPE,SIDE) fabs(a.SIDE),
		DISTANCES_ITEMS(X)
		#undef X
	};
	return pos;
}


Drivebase::Distances operator-(Drivebase::Distances const& a){
	Drivebase::Distances opposite = {
		#define X(TYPE,SIDE) -a.SIDE,
		DISTANCES_ITEMS(X)
		#undef X
	};
	return opposite;
}


Drivebase::Distances operator-(Drivebase::Distances const& a,Drivebase::Distances const& b){
	return a + (-b);
}

Drivebase::Encoder_ticks::Encoder_ticks():l(0),r(0){}
Drivebase::Distances::Distances():l(0),r(0){}
Drivebase::Distances::Distances(double d):l(d),r(d){}

IMPL_STRUCT(Drivebase::Encoder_ticks::Encoder_ticks,ENCODER_TICKS)
IMPL_STRUCT(Drivebase::Speeds::Speeds,SPEEDS_ITEMS)
IMPL_STRUCT(Drivebase::Distances::Distances,DISTANCES_ITEMS)

IMPL_STRUCT(Drivebase::Status::Status,DRIVEBASE_STATUS)
IMPL_STRUCT(Drivebase::Input::Input,DRIVEBASE_INPUT)
IMPL_STRUCT(Drivebase::Output::Output,DRIVEBASE_OUTPUT)

CMP_OPS(Drivebase::Encoder_ticks,ENCODER_TICKS)
CMP_OPS(Drivebase::Speeds,SPEEDS_ITEMS)

CMP_OPS(Drivebase::Input,DRIVEBASE_INPUT)

CMP_OPS(Drivebase::Status,DRIVEBASE_STATUS)

set<Drivebase::Status> examples(Drivebase::Status*){
	return {Drivebase::Status{
		array<Motor_check::Status,Drivebase::MOTORS>{
			Motor_check::Status::OK_,
			Motor_check::Status::OK_
		}
		,
		false,
		{0.0,0.0},
		{0,0},
		*examples((Drivebase::Output*)nullptr).begin(),
		0.0,
		0.0
	}};
}

set<Drivebase::Goal> examples(Drivebase::Goal*){
	return {
		Drivebase::Goal::rotate(0),
		Drivebase::Goal::drive_straight(0),
		Drivebase::Goal::distances({0,0}),
		Drivebase::Goal::absolute(0,0),
		Drivebase::Goal::absolute(1,1)
	};
}

std::ostream& operator<<(std::ostream& o, Drivebase::Goal::Mode a){
	#define X(name) if(a==Drivebase::Goal::Mode::name) return o<<""#name;
	DRIVEBASE_GOAL_MODES
	#undef X
	nyi
}

Drivebase::Goal::Goal():mode_(Drivebase::Goal::Mode::ABSOLUTE),distances_({0,0}),left_(0),right_(0){}

Drivebase::Goal::Mode Drivebase::Goal::mode()const{
	return mode_;
}

Drivebase::Distances Drivebase::Goal::distances()const{
	assert(mode_ == Drivebase::Goal::Mode::DISTANCES);
	return distances_;
}

double Drivebase::Goal::right()const{
	assert(mode_ == Drivebase::Goal::Mode::ABSOLUTE);
	return right_;
}

double Drivebase::Goal::left()const{
	assert(mode_ == Drivebase::Goal::Mode::ABSOLUTE);
	return left_;
}

double Drivebase::Goal::target_distance()const{
	assert(mode_ == Drivebase::Goal::Mode::DRIVE_STRAIGHT);
	return target_distance_;
}

Rad Drivebase::Goal::angle()const{
	assert(mode_ == Drivebase::Goal::Mode::ROTATE);
	return angle_;
}

Drivebase::Goal Drivebase::Goal::distances(Drivebase::Distances distances){
	Drivebase::Goal a;
	a.mode_ = Drivebase::Goal::Mode::DISTANCES;
	a.distances_ = distances;
	return a;
}

Drivebase::Goal Drivebase::Goal::absolute(double left,double right){
	Drivebase::Goal a;
	a.mode_ = Drivebase::Goal::Mode::ABSOLUTE;
	a.left_ = left;
	a.right_ = right;
	return a;
}

Drivebase::Goal Drivebase::Goal::drive_straight(double target){
	Drivebase::Goal a;
	a.mode_ = Drivebase::Goal::Mode::DRIVE_STRAIGHT;
	a.target_distance_ = target;
	return a;
}

Drivebase::Goal Drivebase::Goal::rotate(Rad angle){
	Drivebase::Goal a;
	a.mode_ = Drivebase::Goal::Mode::ROTATE;
	a.angle_ = angle;
	return a;
}

ostream& operator<<(ostream& o,Drivebase::Goal const& a){
	o<<"Drivebase::Goal("<<a.mode()<<" ";
	switch(a.mode()){
		case Drivebase::Goal::Mode::ROTATE:
			o<<a.angle();
			break;
		case Drivebase::Goal::Mode::DRIVE_STRAIGHT:
			o<<a.target_distance();
			break;
		case Drivebase::Goal::Mode::DISTANCES:
			o<<a.distances();
			break;
		case Drivebase::Goal::Mode::ABSOLUTE:
			o<<a.left()<<" "<<a.right();
			break;
		default: 
			nyi
	}
	o<<")";
	return o;
}

#define CMP(name) if(a.name<b.name) return 1; if(b.name<a.name) return 0;

bool operator<(Drivebase::Goal const& a,Drivebase::Goal const& b){
	CMP(mode())
	CMP(left())
	CMP(right())
	return 0;
}

CMP_OPS(Drivebase::Output,DRIVEBASE_OUTPUT)

set<Drivebase::Output> examples(Drivebase::Output*){
	return {
		Drivebase::Output{0,0},
		Drivebase::Output{1,1}
	};
}

set<Drivebase::Input> examples(Drivebase::Input*){
	auto d=Digital_in::_0;
	auto p=make_pair(d,d);
	return {Drivebase::Input{
		{0,0,0,0},p,p,{0,0},0.0
	}};
}

Drivebase::Estimator::Estimator():motor_check(),last({{{}},false,{0,0},{0,0},{0,0},0.0,0.0}){}

Drivebase::Status_detail Drivebase::Estimator::get()const{
	/*array<Motor_check::Status,MOTORS> a;
	for(unsigned i=0;i<a.size();i++){
		a[i]=motor_check[i].get();
	}*/
	
	return last;//Status{a,stall,piston/*,speeds,last_ticks*/};
}

ostream& operator<<(ostream& o,Drivebase::Output_applicator){
	return o<<"output_applicator";
}

ostream& operator<<(ostream& o,Drivebase const& a){
	return o<<"Drivebase("<<a.estimator.get()<<")";
}

double get_output(Drivebase::Output out,Drivebase::Motor m){
	#define X(NAME,POSITION) if(m==Drivebase::NAME) return out.POSITION;
	X(LEFT1,l)
	X(LEFT2,l)
	X(RIGHT1,r)
	X(RIGHT2,r)
	#undef X
	assert(0);
}

void Drivebase::Estimator::update(Time now,Drivebase::Input in,Drivebase::Output out){
	last.dt = now - last.now;//TODO: should now come from input?
	last.now = now;
	last.last_output = out;
	
	speed_timer.update(now,true);
	static const double POLL_TIME = .05;//seconds
	if(speed_timer.done()){
		last.speeds.l = (last.distances.l-in.distances.l)/POLL_TIME;
		last.speeds.r = (last.distances.r-in.distances.r)/POLL_TIME;
		speed_timer.set(POLL_TIME);
	}
	
	last.distances = in.distances;

	for(unsigned i=0;i<MOTORS;i++){
		Drivebase::Motor m=(Drivebase::Motor)i;
		auto current=in.current[i];
		auto set_power_level=get_output(out,m);
		motor_check[i].update(now,current,set_power_level);
	}
	/*
	static const double STALL_CURRENT = .30;//from testing with autonomous
	static const double STALL_SPEED = .10;//ft/s speed at which we assume robot is stalled when current spikes
	last.stall = mean(in.current) > STALL_CURRENT && mean(fabs(last.speeds.l),fabs(last.speeds.r)) < STALL_SPEED;
	cout<<"curr:"<<mean(in.current)<<" "<<mean(last.speeds.l,last.speeds.r)<<"\n";
	*/
	stall_monitor.update(mean(in.current),mean(fabs(last.speeds.l),fabs(last.speeds.r)));
	if(stall_monitor.get()) last.stall = *(stall_monitor.get());
}

Robot_outputs Drivebase::Output_applicator::operator()(Robot_outputs robot,Drivebase::Output b)const{
	robot.talon_srx[L_MOTOR_LOC_1].power_level = b.l;
	robot.talon_srx[L_MOTOR_LOC_2].power_level = b.l;
	robot.talon_srx[R_MOTOR_LOC_1].power_level = -b.r;
	robot.talon_srx[R_MOTOR_LOC_2].power_level = -b.r;//reverse right side for software dev bot 2017

	auto set_encoder=[&](unsigned int a, unsigned int b,unsigned int loc){
		robot.digital_io[a] = Digital_out::encoder(loc,1);
		robot.digital_io[b] = Digital_out::encoder(loc,0);
	};
	
	set_encoder(L_ENCODER_PORTS,L_ENCODER_LOC);
	set_encoder(R_ENCODER_PORTS,R_ENCODER_LOC);
	/*robot.digital_io[0]=Digital_out::encoder(0,1);
	robot.digital_io[1]=Digital_out::encoder(0,0);
	robot.digital_io[2]=Digital_out::encoder(1,1);
	robot.digital_io[3]=Digital_out::encoder(1,0);
	robot.digital_io[4]=Digital_out::encoder(2,1);
	robot.digital_io[5]=Digital_out::encoder(2,0);*/

	robot.digital_io[10] = Digital_out::one();

	return robot;
}

Drivebase::Output Drivebase::Output_applicator::operator()(Robot_outputs robot)const{
	//assuming both motors on the same side are set to the same value//FIXME ?
	return Drivebase::Output{	
		robot.talon_srx[L_MOTOR_LOC_1].power_level,
		-robot.talon_srx[R_MOTOR_LOC_1].power_level, //reverse right side for software dev bot 2017
	};
}

bool operator==(Drivebase::Output_applicator const&,Drivebase::Output_applicator const&){
	return true;
}

bool operator==(Drivebase::Estimator const& a,Drivebase::Estimator const& b){
	if(a.last != b.last) return false;
	if(a.speed_timer != b.speed_timer) return false;
	/*for(unsigned i=0; i<Drivebase::MOTORS; i++){
		if(a.motor_check[i]!=b.motor_check[i])return false;
	}*/
	return true;
}

bool operator!=(Drivebase::Estimator const& a,Drivebase::Estimator const& b){
	return !(a==b);
}

bool operator==(Drivebase const& a,Drivebase const& b){
	return a.estimator==b.estimator && a.output_applicator==b.output_applicator;
}

bool operator!=(Drivebase const& a,Drivebase const& b){
	return !(a==b);
}

//TODO: Rename units
Drivebase::Output trapezoidal_speed_control(Drivebase::Status status, Drivebase::Goal goal){
	Drivebase::Output out = {0,0};
	const double MAX_OUT = 1.0;//in "volts"
	{//for ramping up (based on time)
		const double SPEED_UP_TIME = 2000; //milliseconds
		const double SLOPE = MAX_OUT / SPEED_UP_TIME; //"volts"/ms //TODO: currently arbitrary value
		const double MAX_STEP = 0.2;//"volts" //TODO: currently arbitrary value
		const double MILLISECONDS_PER_SECONDS = 1000 / 1;
		
		double step = clamp(status.dt * MILLISECONDS_PER_SECONDS * SLOPE,-MAX_STEP,MAX_STEP);// in "volts" 
		double l_step = copysign(step,goal.distances().l);
		double r_step = copysign(step,goal.distances().r);
		
		//cout<<"\ndt:"<<status.dt * MILLISECONDS_PER_SECONDS<<" ms step:"<<step<<" "<<status<<"\n";
		
		out = {clamp(status.last_output.l + l_step,-MAX_OUT,MAX_OUT),clamp(status.last_output.r + r_step,-MAX_OUT,MAX_OUT)};
	}	
	{//for rampping down (based on distance)
		Drivebase::Distances error = goal.distances() - status.distances;
		const double SLOW_WITHIN_DISTANCE = 50; //inches
		const double SLOPE = MAX_OUT / SLOW_WITHIN_DISTANCE; //"volts"/inches //TODO: currently arbitrary value
		
		if(error.l < SLOW_WITHIN_DISTANCE)
			out.l = clamp((error.l * SLOPE), -MAX_OUT, MAX_OUT);
	
		if(error.r < SLOW_WITHIN_DISTANCE)
			out.r = clamp((error.r * SLOPE), -MAX_OUT, MAX_OUT);
	}
	
	return out;
}

Drivebase::Output control(Drivebase::Status status,Drivebase::Goal goal){
	switch(goal.mode()){
		case Drivebase::Goal::Mode::DISTANCES:
			return trapezoidal_speed_control(status,goal);
		case Drivebase::Goal::Mode::ABSOLUTE:
			return Drivebase::Output{goal.left(),goal.right()};
		case Drivebase::Goal::Mode::DRIVE_STRAIGHT:
			nyi //TODO
		case Drivebase::Goal::Mode::ROTATE:
			nyi //TODO
		default:
			nyi
	}
}

Drivebase::Status status(Drivebase::Status a){ return a; }

bool ready(Drivebase::Status status,Drivebase::Goal goal){
	switch(goal.mode()){
		case Drivebase::Goal::Mode::ABSOLUTE:
			return true;
		case Drivebase::Goal::Mode::DISTANCES:
			{
				const double TOLERANCE = 1;//inches
				return fabs(goal.distances().l - status.distances.l) < TOLERANCE;//TODO: this is just a placeholder for now
			}
		case Drivebase::Goal::Mode::DRIVE_STRAIGHT:
			{
				const double TOLERANCE = 1;//inches
				double left_error = fabs(goal.target_distance() - status.distances.l),
					right_error = fabs(goal.target_distance() - status.distances.r);
				return ((left_error + right_error) / 2) < TOLERANCE;
			}
		case Drivebase::Goal::Mode::ROTATE:
			nyi //TODO
		default:
			nyi
	}
}

#ifdef DRIVEBASE_TEST
#include "formal.h"
#include <unistd.h>
#include <fstream>

//temp
using Inch = double;

static const Inch ROBOT_WIDTH = 28; //inches, ignores bumpers //TODO: finds some way of dealing with constants like this and wheel diameter

struct Drivebase_sim{
	using Input=Drivebase::Input;
	using Output=Drivebase::Output;
	
	Point position; //x,y are in distance in feet, theta is in radians (positive is counterclockwise from straight forward)
	Time last_time;
	Drivebase::Distances distances;

	void update(Time t,bool enable,Output out){
		static const double POWER_TO_SPEED = 6.5 * 12;//speed is in/s assuming low gear
		Time dt=t-last_time;
		last_time=t;
		if(!enable) return;
		Drivebase::Speeds speeds = {out.l * POWER_TO_SPEED, out.r * POWER_TO_SPEED};
		Drivebase::Distances ddistances = {speeds.l * dt, speeds.r * dt};
		double avg_dist_traveled = mean(ddistances.l,ddistances.r);
		double dtheta = ((out.l-out.r)*POWER_TO_SPEED*dt)/ROBOT_WIDTH;
		/*
			Angle is calculated as the difference between the two sides' powers divided by 2 --   (out.l - out.r) / 2
			That is then converted to a distance   --   ((out.l - out.r) / 2) * POWER_TO_SPEED * dt
			That distance is then converted to an angle -- ((((out.l - our.r) / 2) * POWER_TO_SPEED * dt) * 2) / ROBOT_WIDTH
		*/
		double dy = avg_dist_traveled * cosf(position.theta);
		double dx = avg_dist_traveled * sinf(position.theta);
		distances += ddistances;
		position += {dx,dy,dtheta};
	}
	Input get()const{
		auto d = Digital_in::_0;
		auto p = make_pair(d,d);
		Drivebase::Input in = {Drivebase::Input{
			{0,0,0,0},p,p,distances,0.0
		}};
		return in;
	}

	Drivebase_sim():position({}),last_time(0),distances({0,0}){}

};

ostream& operator<<(ostream& o,Drivebase_sim const& a){
	return o << "Drivebase_sim(" << a.position << ")";
}

///end temp

int main(){
	{
		Drivebase d;//TODO: re-enable
		tester(d);
	}
	/*
	{
		Drivebase::Encoder_ticks a = {100,100}, b = {10,10};
		Drivebase::Encoder_ticks diff = a - b, sum = a + b, opp = -a;
		cout<<"\na:"<<a<<" b:"<<b<<" diff:"<<diff<<" sum:"<<sum<<" opp:"<<opp<<"\n";
	}
	*/
	{
		cout<<"\n==========================================================\n";
		Drivebase_sim drive_sim;
		Drivebase drive;
		const Time TIMESTEP = .020;//sec
		const Time MAX_TEST_LENGTH = 15;//sec
		
		Drivebase::Goal goal = Drivebase::Goal::distances({100,100});//inches
		
		const bool ENABLED = true;
		
		for(Time t = 0; t < MAX_TEST_LENGTH; t += TIMESTEP){
			Drivebase::Status_detail status = drive.estimator.get();
			Drivebase::Output out = control(status,goal);
			
			drive_sim.update(t,ENABLED,out);
			
			Drivebase::Input input = drive_sim.get();
			
			drive.estimator.update(t,input,out);
			
			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";
			
			if(ready(status,goal)){
				cout<<"t:"<<t<<"\tgoal "<<goal<<" reached with status "<<status<<".\nFinishing\n";
				break;
			}
		}
	}
}
#endif
