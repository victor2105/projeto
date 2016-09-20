#include "Fight.h"

/**
* Atacar:
* Causar dano em uma unidade inimiga
* Inicio - Encontrar - Atacar - Fim
*/

Fight::~Fight()
{
}



void Fight::setUnit(Unit nu){
	u = nu;
	if (u->getType() == UnitTypes::Terran_Vulture)
		unitDamage = 20;
}



void Fight::setMaxRange(int mr){
	MaxRange = mr;
}



void Fight::start(){
	_finished = false;
}



void Fight::update(){

	// Ferificar condição de ataque
			// Atualizar ataque
		Unitset enemies = u->getUnitsInRadius(MaxRange * 32, BWAPI::Filter::IsEnemy);
		Unit enemy = nullptr;

		if (u->getKillCount() > lastKill) {
			positions[_lastEnemySelectedID].hp = 0;
			lastKill = u->getKillCount();
		}
		
		int health = 9999;
		// Atacar inimigo com menos vida
		for (auto &m : enemies)
		{
			if (m->getHitPoints() < health){
				health = m->getHitPoints();
				enemy = m;
			}
			updatePosition(m->getID(), m);
			Broodwar->drawCircleMap(m->getPosition(), 15, BWAPI::Colors::Blue, false);
		}

		if (_finished)
			return;

		if (enemy){
			Broodwar->drawCircleMap(enemy->getPosition(), 15, BWAPI::Colors::Green, true);
			u->attack(enemy);
			_lastEnemySelectedID = enemy->getID();
		}
		else{
			
			Unitset enemies2 = u->getUnitsInRadius(MaxRange * 32 * 2, BWAPI::Filter::IsEnemy);
			Unit enemy2 = nullptr;


			int health = 9999;
			// Atacar inimigo com menos vida
			for (auto &m : enemies2)
			{
				updatePosition(m->getID(), m);
				Broodwar->drawCircleMap(m->getPosition(), 15, BWAPI::Colors::Blue, false);
				Broodwar->drawTextMap(m->getPosition().x - 15, m->getPosition().y + 30, "id: %d\nHP: %d", m->getID(), m->getHitPoints());
			}
			
			Position p = getClosestAlive(u->getPosition());		
			if (p.x == 0 && p.y == 0){
				p = getClosest(u->getPosition());
				Broodwar->drawCircleMap(p, 15, BWAPI::Colors::Red, true);				
			}
			else {
				Broodwar->drawCircleMap(p, 15, BWAPI::Colors::Red, false);
			}
			u->attack(p);
		}
	
}



void Fight::end(){
	// Atualizar a vida do ultimo inimigo selectionado
//	cout << "END FIGHT" << endl;
	updateHit();
	_canShot = false;
	_finished = true;
	u->stop();
	ActionImp::getInstance().actionFinished = true;
	ActionImp::getInstance().action = ActionImp::NO_ACTION;
}



bool Fight::updateHit(){
	
	positions[_lastEnemySelectedID].hp -= unitDamage;
	if (positions[_lastEnemySelectedID].hp <= 0){
		positions[_lastEnemySelectedID].hp = 0;
	}
	return true;
}



bool Fight::hasFinished(){
	return _finished;
}



void Fight::execute(){
	
}



void Fight::clear(){
	positions.clear();
	u = nullptr;
	_canShot = true;
	_finished = true;
	lastKill = 0;
}



void Fight::addPosition(int id, Unit p){
	myUnit m;
	m.id = p->getID();
	m.hp = p->getHitPoints();
	m.p = p->getPosition();
	positions[id] = m;
//	cout << "add " << m.id << "," << m.hp << "," << m.p << endl;
}



void Fight::updatePosition(int id, Unit p){
	if (positions.count(id) > 0){
		myUnit m;
		m.id = p->getID();
		m.hp = p->getHitPoints();
		m.p = p->getPosition();
		positions[id] = m;
	}else{
		addPosition(id, p);
	}
}



Position Fight::getClosestAlive(Position p){
	
	float dist_min;
	int id = -1;
	int hp = 0;
	float d = 99999999;
	Position ep;


	for (itp_type i = positions.begin(); i != positions.end(); i++) {
		if (i->second.hp != 0){
			if (d > u->getPosition().getDistance(i->second.p)) {
				d = u->getPosition().getDistance(i->second.p);
				id = i->first;
				ep = i->second.p;
				hp = i->second.hp;
				if (u->getPosition().getDistance(ep) <= MaxRange * 32){
					positions.erase(i);
				}
			}
		}
	}
	if (id != -1) {
		Broodwar->drawTextMap(ep.x - 30, ep.y + 50, "id: %d\nHP: %d", id, hp);

		_lastEnemySelectedID = id;

		

		return ep;
	}
	return getPosition(id);
}



Position Fight::getClosest(Position p){

	float dist_min;
	int id = -1;
	float d;

	for (itp_type i = positions.begin(); i != positions.end(); i++) {
		d = p.getDistance(i->second.p);
		if (id == -1 || dist_min > d || positions[id].hp == 0 || u->getPosition().getDistance(i->second.p) > 5){
			dist_min = d;
			id = i->first;
		}
	}
	
	return getPosition(id);
}



Position Fight::getPosition(int id){
	if (positions.count(id)){
		Broodwar->drawTextMap(positions[id].p + Position(0, -20), "id = %d\nhp = %d", positions[id].id, positions[id].hp);
		if (positions[id].hp == 0 || u->getPosition().getDistance(positions[id].p) < 5 ){
			positions[id].time=1;
		}
		if (positions[id].p.getDistance(u->getPosition()) < 5){
			return u->getPosition() + Position(64, 0);

		}
		return positions[id].p;
	}
	Position p = u->getPosition() + Position(64, 0);
	return p;
}



map<int, myUnit> * Fight::getMyUnits(){
	return &positions;
}