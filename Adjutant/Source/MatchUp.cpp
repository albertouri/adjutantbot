#include <iostream>
#include <fstream>
#include "MatchUp.h"
#include "AdjutantAIModule.h"


MatchUp::MatchUp(void)
{
}

MatchUp::MatchUp(BWAPI::Unit* myUnit, BWAPI::Unit* enemyUnit)
{
	this->myUnit = myUnit;
	this->enemyUnit = enemyUnit;
	this->myUnitType = myUnit->getType();
	this->enemyUnitType = enemyUnit->getType();
	this->myUnitInitialHitPoints = myUnit->getInitialHitPoints();
	this->enemyUnitInitialHitPoints = enemyUnit->getInitialHitPoints();
}

private void printResults(std::ofstream& file)
{
	if (file.is_open())
	{
		// This will cause problems if dividing by 0 ever happens
		//float matchUpComparsion = (float) (enemyUnitInitial - enemyUnitFinal) / (myUnitInitial - myUnitFinal);
	}
	else
	{
		//cout << "Unable to open file";
	}
}

MatchUp::~MatchUp(void)
{
}