#pragma once
#include "action.h"
#include <BWAPI.h>
#include <sstream>

class ConstructBuildingAction : public Action
{
public:
	ConstructBuildingAction(int priority, BWAPI::TilePosition loc, BWAPI::UnitType unitType); 
	~ConstructBuildingAction(void);
	bool isReady();
	bool isStillValid();
	void execute();
	std::string toString();
protected:
	BWAPI::TilePosition location;
	BWAPI::UnitType buildingType;
};
