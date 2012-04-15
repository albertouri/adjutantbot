#pragma once
#include <BWAPI.h>
#include <vector>

class WorldModel
{
public:
	WorldModel(void);
	~WorldModel(void);

	void update();

	//*****Our Units*****
	
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
	//Potential Expansions

private:
	void handleOurUnitCompleted(BWAPI::Unit* unit);
};
