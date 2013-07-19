#pragma once
#include <BWAPI.h>
#include <queue>
#include "Threat.h"
#include "Utils.h"
#include "WorldManager.h"

class MilitaryManager
{
public:
	MilitaryManager(void);
	~MilitaryManager(void);
	void evalute();
	void manageFightingWorkers();
	void manageVultureMining();
	void manageUnitAbilities(BWAPI::Unit* unit, bool* isUnitToRemove);
	//void manageUnitAbilities(BWAPI::Unit* unit, BWAPI::Unit* closestEnemy);
	void manageDefense();
};
