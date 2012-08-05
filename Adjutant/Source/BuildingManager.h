#pragma once
#include <queue>
#include <BWAPI.h>
#include "Action.h"
#include "ActionComparator.h"
#include "ConstructBuildingAction.h"
#include "WorldModel.h"

class BuildingManager
{
public:
	BuildingManager(void);
	~BuildingManager(void);
	static void evalute(WorldModel* worldModel, ActionQueue* actionQueue);
};
