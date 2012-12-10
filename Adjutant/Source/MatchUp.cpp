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
	this->myUnitInitialHitPoints = myUnit->getHitPoints();
	this->enemyUnitInitialHitPoints = enemyUnit->getHitPoints();
}

MatchUp::~MatchUp(void)
{
}