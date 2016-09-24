#include "EnemiesHealth.h"

EnemiesHealth::~EnemiesHealth()
{
}


void EnemiesHealth::setUnit(Unit nu){
	u = nu;
}



int EnemiesHealth::getDeltaHealth() const{
	return delta;
}



void EnemiesHealth::clean(){
	delta = 0;
	deltaKills = 0;
}


void EnemiesHealth::reset(){
	clean();
	mapIDHealth.clear();
	killsNumber = 0;
}

void EnemiesHealth::calculateDeltahHealth(){
	/*
	map<int, myUnit> * enemiesM = Fight::getInstance().getMyUnits();
	for (map<int, myUnit>::iterator e = enemiesM->begin(); e != enemiesM->end(); e++){
		int id = e->first;
		if (mapIDHealth.count(id) > 0){ // id existe
			delta += mapIDHealth[id] - e->second.hp;
			updateEnemyUnit(id, e->second.hp);
		}
		else{
			addEnimyUnit(id, e->second.hp);
		}
	}
	/**/
	Unitset enemies = u->getUnitsInRadius(200, BWAPI::Filter::IsEnemy);
	deltaKills = u->getKillCount() - killsNumber;
	killsNumber = u->getKillCount();

	for (auto & e : enemies){
		int id = e->getID();
		if (mapIDHealth.count(id) > 0){
			delta += mapIDHealth[id] - e->getHitPoints();
			if (e->getHitPoints() == 0)
				removeEnemyUnit(id);
			else
				updateEnemyUnit(id, e->getHitPoints());
		}
		else{
			addEnimyUnit(id, e->getHitPoints());
		}
	}

	//delta += deltaKills * 20;
	
	/**/

}



void EnemiesHealth::addEnimyUnit(int id, int health){
	mapIDHealth[id] = health;
}



void EnemiesHealth::removeEnemyUnit(int id){
	if (mapIDHealth.count(id) > 0)
		mapIDHealth.erase(id);
}



void EnemiesHealth::updateEnemyUnit(int id, int health){
	mapIDHealth[id] = health;
}