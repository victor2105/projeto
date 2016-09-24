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
	/**/
	Unitset enemies = u->getUnitsInRadius(10000, BWAPI::Filter::IsEnemy);
	deltaKills = u->getKillCount() - killsNumber;
	killsNumber = u->getKillCount();

	for (auto & e : enemies){
		updateEnemyUnit(e->getID() , e->getHitPoints());
	}	
	/**/
}



void EnemiesHealth::unitShow(int id, int health){
	updateEnemyUnit(id, health);
}



void EnemiesHealth::unitDestroy(int id){
	if (mapIDHealth.count(id) > 0){
		updateEnemyUnit(id, 0);
		mapIDHealth.erase(id);
	}
}

void EnemiesHealth::unitHide(int id, int health){
	if (mapIDHealth.count(id) > 0){
		//updateEnemyUnit(id, health);
	}
}


void EnemiesHealth::updateEnemyUnit(int id, int health){
	if (mapIDHealth.count(id) == 0){
		mapIDHealth[id] = health;
	}
	else{
		if (health < mapIDHealth[id]){
			delta += mapIDHealth[id] - health;
			cout << "Unidade: " << id << "Perdeu " << mapIDHealth[id] - health << "de vida." << endl;
			mapIDHealth[id] = health;
		}
	}
}