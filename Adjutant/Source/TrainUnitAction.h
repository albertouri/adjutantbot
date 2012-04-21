#pragma once
#include "action.h"
#include <BWAPI.h>
#include <sstream>

class TrainUnitAction : public Action
{
public:
	TrainUnitAction(int priority, BWAPI::Unit* building, BWAPI::UnitType unitType);
	~TrainUnitAction(void);
	bool isReady();
	bool isStillValid();
	void execute();
	std::string toString();
protected:
	BWAPI::Unit* building;
	BWAPI::UnitType unitType;
};
