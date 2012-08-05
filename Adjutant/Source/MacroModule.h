#pragma once
#include "ActionQueue.h"
#include <BWAPI.h>
#include "BuildingManager.h"
#include "ConstructBuildingAction.h"
#include <queue>
#include <time.h>
#include "TrainUnitAction.h"
#include "WorldModel.h"

class MacroModule
{
public:
	MacroModule(void);
	~MacroModule(void);
	void evalute(WorldModel* worldModel, ActionQueue* actionQueue);
};
