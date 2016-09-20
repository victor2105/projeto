#pragma once
#include "ActionImp.h"
#include <BWAPI.h>

using namespace BWAPI;
class Reatreat
{
public:
	Reatreat(Unit nu = 0):u(nu) {
		_finished = true;
	}
	~Reatreat();
	void execute();
	bool hasFinished();
	void setUnit(Unit nu);
	void clear();
private:
	void start();
	void update();
	void end();
	Unit u;
	Position nextPosition;
	bool _finished;
};

