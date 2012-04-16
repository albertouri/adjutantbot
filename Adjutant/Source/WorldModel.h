#pragma once
#include <BWAPI.h>
#include "BWTA.h"
#include "UnitGroup.h"
#include "Utils.h"
#include <vector>

class WorldModel
{
public:
	WorldModel(void);
	~WorldModel(void);

	void update(bool isTerrainAnalyzed);

	//*****Our Units*****
	
	//Home base region
	BWTA::Region* myHomeRegion;

	//Map of all of our available units based on type
	std::map<BWAPI::UnitType, std::vector<BWAPI::Unit*>*> myUnitMap;

	//Workers - SCVs dedicated to mining minerals
	std::vector<BWAPI::Unit*>* myWorkerVector;

	//Scouts - SCVs or otherwise dedicated to scouting
	std::vector<BWAPI::Unit*>* myScoutVector;

	//Army - all units
	std::vector<BWAPI::Unit*>* myArmyVector;

	//Army - split into groups
	std::vector<UnitGroup*>* myArmyGroups;

	//*****Opponent Model*****
	//Enemy
	BWAPI::Player* enemy;

	//Home base region
	BWTA::Region* enemyHomeRegion;

	//Map of all of our available units based on type
	std::map<BWAPI::UnitType, std::vector<BWAPI::Unit*>*> enemyUnitMap;

	//*****Misc Data****
	//Is BWTA finished?
	bool isTerrainAnalyzed;
};
