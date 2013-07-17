#pragma once
#include <queue>
#include <BWAPI.h>
#include "WorldManager.h"

class InformationManager
{
public:
	InformationManager(void);
	~InformationManager(void);
	void evaluate();
	void manageThreatDetection(); //Find and group enemy threats
	void manageUnitCountering(); //Figure out which unit composition best counters the enemy
	void manageResearch(); //Once we have decided what units to build, figure out what upgrades/abilities we want

private:
	//map[unit type 1][unit type 2] = strength
	//unit type 1 is countered by unit type 2 with strength
	std::map<BWAPI::UnitType, std::map<BWAPI::UnitType, float>> unitCounters;

	void initUnitCounters();
};
