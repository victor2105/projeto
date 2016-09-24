#pragma once


class ActionImp  // Interface class
{
public:
	ActionImp(){
		action = NO_ACTION;
		actionFinished = true;
	}
	virtual ~ActionImp(){} ;
	enum ActionToDo { NO_ACTION=-1, FIGHT=0, REATREAT=1};
	ActionToDo action;
	bool actionFinished;

	static ActionImp & getInstance(){
		static ActionImp instance;
		return instance;
	}

	void clear(){
		action = NO_ACTION;
		actionFinished = true;
	}
};
