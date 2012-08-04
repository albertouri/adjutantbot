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
	void manageThreatDetection();
	void manageUnitCountering();

private:
	//map[unit type 1][unit type 2] = strength
	//unit type 1 is countered by unit type 2 with strength
	std::map<BWAPI::UnitType, std::map<BWAPI::UnitType, float>> unitCounters;

	void initUnitCounters();
};
