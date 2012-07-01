#pragma once
#include <BWAPI.h>
#include "BWTA.h"
#include "ConstructBuildingAction.h"
#include "HistoricalUnitInfo.h"
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

	//Home base location
	BWTA::BaseLocation* myHomeBase;

	//Map of all of our available units based on type
	std::map<BWAPI::UnitType, std::vector<BWAPI::Unit*>> myUnitMap;

	//Workers - SCVs dedicated to mining minerals
	std::vector<BWAPI::Unit*>* myWorkerVector;

	//Scouts - SCVs or otherwise dedicated to scouting
	std::vector<BWAPI::Unit*>* myScoutVector;

	//Army - all units
	std::vector<BWAPI::Unit*>* myArmyVector;

	//Army - split into groups
	std::vector<UnitGroup*>* myArmyGroups;

	//Workers that are on their way to build a building
	std::map<BWAPI::Unit*, ConstructBuildingAction*> workersBuildingMap;

	//Reserved minerals
	int reservedGas;

	//Reserved gas
	int reservedMinerals;

	//*****Opponent Model*****
	//Enemy
	BWAPI::Player* enemy;

	//Home base region
	BWAPI::Region* enemyHomeRegion;

	//Map of all available enemy units based on type -- only enemies currently on screen
	std::map<BWAPI::UnitType, std::vector<BWAPI::Unit*>> enemyUnitMap;

	//Map of all available enemy units we have ever seen and their last known location
	std::map<int, HistoricalUnitInfo> enemyHistoricalUnitMap;

	//*****Misc Data****
	//Is BWTA finished?
	bool isTerrainAnalyzed;

	//Opponent Modeling functions
	int getEnemyArmyValue();
	int getMyArmyValue();
	double getEnemyRangedWeight();
};
