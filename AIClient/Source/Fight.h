#pragma once

#include "ActionImp.h"
#include <BWAPI.h>
#include <map>

using namespace BWAPI;
using namespace std;

typedef struct {
	int id;
	int hp;
	Position p;
	int time;
} myUnit;


typedef map<int, myUnit>::iterator itp_type;


class Fight
{
public:
	Fight(Unit ub = 0, int maxRange = 5) :u(ub), MaxRange(maxRange){
		_started = false;
		_finished = true;
		_canShot = true;
		lastKill = 0;
	}
	~Fight();
	void execute();
	bool hasFinished();
	void setUnit(Unit nu);
	void setMaxRange(int);
	void clear();

	map<int, myUnit> * getMyUnits();

	static Fight & getInstance(){
		static Fight instance;
		return instance;
	}

	bool _canShot;
	void updatePosition(int, Unit);

	void start();
	void update();
	void end();
	int MaxRange;
	int lastSelected;
	int lastKill;
private:

	bool updateHit();
	Unit selecioneInimigo(int distance);

	map<int, myUnit> positions;
	Unit u;
	int unitDamage;
	int lifeTime = 0;
	void addPosition(int, Unit);
	Position getPosition(int);
	Position getClosest(Position);
	Position getClosestAlive(Position);
	bool _started;
	bool _finished;
	int _lastEnemySelectedID;

	
};


