#pragma once

#include <map>
#include <BWAPI.h>
#include "Fight.h"
#include <iostream>

using namespace BWAPI;
using namespace std;

// A ideia eh calcular o delta health para as unidades visiveis e supor que as unidades
// fora do campo de visao estejam com a mesma vida;

class EnemiesHealth
{
public:
	EnemiesHealth(Unit nu = 0):u(nu){}

	~EnemiesHealth();

	int getDeltaHealth() const;
	void calculateDeltahHealth();
	void setUnit(Unit nu);
	void clean();
	void reset();

	void unitShow(int, int);
	void unitHide(int, int);
	void unitDestroy(int);


	static EnemiesHealth & getInstance(){
		static EnemiesHealth instance;
		return instance;
	}
private:
	Unit u;
	map<int, int> mapIDHealth;
	int delta;
	int killsNumber;
	int deltaKills;

	void updateEnemyUnit(int, int);
	
};

