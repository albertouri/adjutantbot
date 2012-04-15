#pragma once
#include <BWAPI.h>
#include "BWTA.h"
#include <vector>

class WorldModel
{
public:
	WorldModel(void);
	~WorldModel(void);

	void update(bool isTerrainAnalyzed);

	//*****Our Units*****
	
	//Home base region
	BWTA::Region* homeRegion;

	//Map of all of our available units based on type
	std::map<BWAPI::UnitType, std::vector<BWAPI::Unit*>*> myUnitMap;

	//Workers - SCVs dedicated to mining minerals
	std::vector<BWAPI::Unit*>* myWorkerVector;

	//Scouts - SCVs or otherwise dedicated to scouting
	std::vector<BWAPI::Unit*>* myScoutVector;

	//Army
	//TODO: one giant mob for now. Will need implement grouping... micro... etc...
	std::vector<BWAPI::Unit*>* myArmyVector;

	//*****Misc Data****
	//Is BWTA finished?
	bool isTerrainAnalyzed;

	void handleOurUnitCreated(BWAPI::Unit* unit);

};
