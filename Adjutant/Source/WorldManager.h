#pragma once
#include "Base.h"
#include <BWAPI.h>
#include "BWTA.h"
#include "BuildTask.h"
#include "ConstructBuildingAction.h"
#include "HistoricalUnitInfo.h"
#include "Threat.h"
#include "UnitGroup.h"
#include "Utils.h"
#include <vector>

class WorldManager
{
public:
	static WorldManager& Instance()
	{
		static WorldManager instance;
		return instance;
	}

	~WorldManager(void);

	void update(bool isTerrainAnalyzed);


	//*****Our Units*****

	//Base locations (set in UnitManager)
	Base* myHomeBase;
	std::vector<Base*> myBaseVector;

	//List of regions of the map we want to keep clear of enemy units
	std::vector<BWTA::Region*> protectedRegionVector;

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

	//Map of unit types that workers are on their way to build
	std::map<BWAPI::UnitType, int> imminentBuildingMap;

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

	//List of known threats (groups of enemy combatants)
	std::vector<Threat*> threatVector;

	//*****Misc Data****
	//Is BWTA finished?
	bool isTerrainAnalyzed;

	//Structure used to pass data between UnitManager and BuildManager
	std::vector<BuildTask*> buildTaskVector;

	//Updating current bases
	void checkForBases();
	void addBase(BWAPI::Unit* commandCenter);

	//Opponent Modeling functions
	int getEnemyArmyValue();
	static int getMyArmyValue();
	static int getMyAttackValue();
	double getEnemyRangedWeight();

private:
	WorldManager(void);
	WorldManager(const WorldManager&);
	WorldManager& operator=(const WorldManager&);
};
