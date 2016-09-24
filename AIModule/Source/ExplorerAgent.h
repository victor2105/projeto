#pragma once

#include <BWAPI.h>

using namespace BWAPI;

class ExplorerAgent
{
private:
	Unit u;

public:
	ExplorerAgent();
	~ExplorerAgent();

	void setUnit(Unit nu);



	void execute();

};

