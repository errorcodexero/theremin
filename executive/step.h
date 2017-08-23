#ifndef STEP_H
#define STEP_H

#include "executive.h"
#include "../util/motion_profile.h"
#include "../util/settable_constant.h"

struct Step_impl;

class Step{
	public:
	enum class Status{UNFINISHED,FINISHED_SUCCESS,FINISHED_FAILURE};

	private:
	std::unique_ptr<Step_impl> impl;

	public:
	explicit Step(Step_impl const&);
	Step(Step const&);

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Status done(Next_mode_info);
	Step_impl const& get()const;
	void display(std::ostream&)const;
	bool operator==(Step const&)const;
	bool operator<(Step const&)const;
};

std::ostream& operator<<(std::ostream&,Step const&);

struct Step_impl{
	virtual ~Step_impl();

	virtual Toplevel::Goal run(Run_info,Toplevel::Goal)=0;
	virtual Toplevel::Goal run(Run_info)=0;
	virtual Step::Status done(Next_mode_info)=0;//could try to make this const.
	virtual std::unique_ptr<Step_impl> clone()const=0;
	virtual void display(std::ostream&)const;
	virtual bool operator<(Step const&)const=0;
	virtual bool operator==(Step const&)const=0;
};

template<typename T>
struct Step_impl_inner:Step_impl{
	std::unique_ptr<Step_impl> clone()const{
		T const& t=dynamic_cast<T const&>(*this);
		return std::unique_ptr<Step_impl>(new T(t));
	}
	
	void display(std::ostream& o)const{
		o<<type(*(T*)this);
	}

	bool operator==(Step const& a)const{
		//Step_impl const& b=a.get();
		//if(type_index(typeid(*this))!=type_index
		T const& t=dynamic_cast<T const&>(a.get());
		return operator==(t);
	}

	virtual bool operator==(T const&)const=0;

	bool operator<(Step const&)const{
		nyi
	}
};

using Inch=double;

class Drive_straight:public Step_impl_inner<Drive_straight>{//Drives straight a certain distance
	Inch target_dist;
	Drivebase::Distances initial_distances;
	bool init;
	Motion_profile motion_profile;
	Countdown_timer in_range;	
	Countdown_timer stall_timer;
	Gear_shifter::Goal gear;
	
	Drivebase::Distances get_distance_travelled(Drivebase::Distances);//TODO: do this better

	public:
	explicit Drive_straight(Inch);
	explicit Drive_straight(Inch,double,double);

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Drive_straight const&)const;
};

class MP_drive:public Step_impl_inner<MP_drive>{
	Inch target_distance;
	Settable_constant<Drivebase::Goal> drive_goal;
	
	public:
	explicit MP_drive(Inch);

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(MP_drive const&)const;
};

class Ram:public Step_impl_inner<Ram>{//Drives straight a certain distance
	Inch target_dist;
	Drivebase::Distances initial_distances;
	bool init;
	Countdown_timer stall_timer;
	Gear_shifter::Goal gear;

	Drivebase::Distances get_distance_travelled(Drivebase::Distances);//TODO: do this better

	public:
	explicit Ram(Inch);

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Ram const&)const;
};


class Wait: public Step_impl_inner<Wait>{//Either stops all operation for a given period of time or continues to run the same goals for that time
	Countdown_timer wait_timer;//seconds
	public:
	explicit Wait(Time);

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Wait const&)const;
};

class Lift_gear: public Step_impl_inner<Lift_gear>{//Closes the gear grabber and raises the gear collector to peg height
	Gear_collector::Goal gear_goal;//is the same in every one
	public:
	explicit Lift_gear();

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Lift_gear const&)const;
};

class Drop_gear: public Step_impl_inner<Drop_gear>{//Opens the gear grabber but keeps the manipulator at peg height
	Gear_collector::Goal gear_goal;//is the same in every one

	public:
	explicit Drop_gear();

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Drop_gear const&)const;
};

class Drop_collector: public Step_impl_inner<Drop_collector>{//Lowers the gear manipulator to the floor
	Gear_collector::Goal gear_goal;//is the same in every one
	
	public:
	explicit Drop_collector();

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Drop_collector const&)const;
};

class Combo: public Step_impl_inner<Combo>{//Runs two steps at the same time
	Step step_a;
	Step step_b;
	public:
	explicit Combo(Step,Step);//the second step will overwrite goals from the first one if they both modify the same parts of the robot

	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	void display(std::ostream& o)const;
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Combo const&)const;
};

static const Inch ROBOT_WIDTH = 28; //inches, ignores bumpers //TODO: finds some way of dealing with constants like this and wheel diameter

struct Turn: Step_impl_inner<Turn>{//orients the robot to a certain angle relative to its starting orientation
	Rad target_angle;//radians,clockwise=positive
	Drivebase::Distances initial_distances;
	bool init;
	Drivebase::Distances side_goals;
	Motion_profile motion_profile;
	Countdown_timer in_range;

	Drivebase::Distances angle_to_distances(Rad);
	Drivebase::Distances get_distance_travelled(Drivebase::Distances);

	explicit Turn(Rad);
	explicit Turn(Rad,double,double);
	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Turn const&)const;
};

const Inch SCORE_GEAR_APPROACH_DIST = 12.0;//inches


struct Turn_on_light: Step_impl_inner<Turn_on_light>{
	Lights::Goal lights_goal;

	explicit Turn_on_light();
	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Turn_on_light const&)const;
};

struct Score_gear: Step_impl_inner<Score_gear>{
	enum Stage{LIFT,SCORE,RELEASE,BACK_OFF,STOW,DONE};
	std::array<Step,Stage::DONE> steps;
	Stage stage;

	void advance();

	explicit Score_gear();
	Toplevel::Goal run(Run_info,Toplevel::Goal);
	Toplevel::Goal run(Run_info);
	void display(std::ostream& o)const;
	Step::Status done(Next_mode_info);
	std::unique_ptr<Step_impl> clone()const;
	bool operator==(Score_gear const&)const;
};

#endif
