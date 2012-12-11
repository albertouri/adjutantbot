#pragma once
#include <BWAPI.h>
#include "Utils.h"
#include "WorldManager.h"
#include "MatchUp.h"
#include <vector>

class UnitTraining
{
public:
	UnitTraining(void);
	~UnitTraining(void);
	void evalute();
	void printResults(MatchUp* matchUp);

	std::map<BWAPI::Unit*, MatchUp*> matchUpMap;
	std::vector<BWAPI::Unit*>* myUnitVector;
	BWAPI::Unit* heroTrigger;
	BWAPI::UnitType roundTypeUnit;
	bool isTrainerInitialized;
};