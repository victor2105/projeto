#include "ExplorerAgent.h"


ExplorerAgent::ExplorerAgent()
{
}



ExplorerAgent::~ExplorerAgent()
{
}



void ExplorerAgent::setUnit(Unit nu){
	u = nu;
}

void ExplorerAgent::execute(){
	Position p = u->getPosition() + Position(64, 0);
	u->move(p);
	Broodwar->drawCircleMap(p, 15, Colors::Green, false);
}