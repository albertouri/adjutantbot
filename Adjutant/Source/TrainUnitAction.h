#pragma once
#include "ActionQueue.h"
#include <BWAPI.h>
#include <sstream>

class TrainUnitAction : public Action
{
public:
	TrainUnitAction(int priority, BWAPI::Unit* building, BWAPI::UnitType unitType);
	~TrainUnitAction(void);
	bool isReady(int minerals, int gas, int supplyRemaining);
	bool isStillValid();
	void execute();
	void updateResourceCost(int* minerals, int* gas, int* supplyRemaining);
	
	bool operator==(const Action &other) const;
	std::string toString();
protected:
	BWAPI::Unit* building;
	BWAPI::UnitType unitType;
};
