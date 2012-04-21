#pragma once
#include <queue>
#include "Action.h"
#include "ActionComparator.h"
#include <BWAPI.h>
#include "TrainUnitAction.h"
#include "BuildingManager.h"
#include "ConstructBuildingAction.h"
#include "WorldModel.h"

class MacroModule
{
public:
	MacroModule(void);
	~MacroModule(void);
	void evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue);
};
