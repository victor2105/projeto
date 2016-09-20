#include "Reatreat.h"
#include "Fight.h"



Reatreat::~Reatreat()
{
}


void Reatreat::setUnit(Unit nu){
	u = nu;
}

bool Reatreat::hasFinished(){
	return _finished;
}

void Reatreat::start(){
	_finished = false;
	Unitset enemies = u->getUnitsInRadius(200, BWAPI::Filter::IsEnemy);
	Unit closestEnemy = nullptr;
	//Broodwar->sendText("REATREAT ACTION");

	for (auto &m : enemies)
	{
		if (!closestEnemy || u->getDistance(m) < u->getDistance(closestEnemy))
			closestEnemy = m;
		Fight::getInstance().updatePosition(m->getID(), m);
	}
	if (closestEnemy){
		Position p = u->getPosition();
		p -= (closestEnemy->getPosition() - p);
		nextPosition = p;
		Broodwar->drawCircleMap(p, 15, BWAPI::Colors::Blue, true);
		u->move(p);
	}
	else{
		Position p = u->getPosition();
		p -= Position(64, 0);
		nextPosition = p;
		Broodwar->drawCircleMap(p, 15, BWAPI::Colors::Blue, true);
		u->move(p);
	}
}

void Reatreat::update(){
	int h = Broodwar->mapHeight() * 32;
	int w = Broodwar->mapWidth() * 32;
	bool positionValidate1 = u->getPosition().x - 32 <= 0 || u->getPosition().y - 32 <= 0;
	bool positionValidate2 = u->getPosition().x + 32 >= w || u->getPosition().y + 32 >= h;
	bool positionValidate3 = nextPosition.x <= 0 || nextPosition.y <= 0 || nextPosition.x >= w || nextPosition.y >= h;
	if (u->getPosition().getApproxDistance(nextPosition) <= 5 || positionValidate1 || positionValidate2 || positionValidate3)
		end();
}

void Reatreat::end(){
	_finished = true;
}


void Reatreat::execute(){
	if (u){
		if (_finished)
			start();
		if (!_finished){
			update();
		}
	}
	else {
		std::cout << "Unit not found\n" << std::endl;
	}
}



void Reatreat::clear(){
	u = nullptr;
	_finished = true;
}
