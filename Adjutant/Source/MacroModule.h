#pragma once
#include <queue>
#include "ActionQueue.h"

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
	void evalute(WorldModel* worldModel, ActionQueue* actionQueue);
};
