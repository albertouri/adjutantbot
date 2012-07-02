#pragma once
#include "ActionQueue.h"
#include <BWAPI.h>
#include <sstream>

//Forward declare to avoid circular reference
class WorldManager;

class ConstructBuildingAction : public Action
{
public:
	ConstructBuildingAction(int priority, BWAPI::TilePosition loc, BWAPI::UnitType unitType); 
	~ConstructBuildingAction(void);
	bool isReady(int minerals, int gas, int supplyRemaining);
	bool isStillValid();
	void execute();
	void updateResourceCost(int* minerals, int* gas, int* supplyRemaining);

	bool operator==(const Action &other) const;
	std::string toString();

	BWAPI::UnitType buildingType;
	BWAPI::TilePosition location;
};
