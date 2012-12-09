#pragma once
#include <BWAPI.h>
#include <vector>
#include <fstream>
#include "Utils.h"
#include "WorldManager.h"

class MatchUp
{
public:
	MatchUp(void);
	MatchUp::MatchUp(BWAPI::Unit* myUnit, BWAPI::Unit* enemyUnit);
	~MatchUp(void);
	void printResults(ofstream& file);

	BWAPI::Unit* myUnit;
	BWAPI::Unit* enemyUnit;
	BWAPI::UnitType myUnitType;
	BWAPI::UnitType enemyUnitType;
	int enemyUnitInitialHitPoints;
	int myUnitInitialHitPoints;
};