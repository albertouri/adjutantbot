#pragma once
#include "ActionQueue.h"
#include <BWAPI.h>
#include "BuildManager.h"
#include "ConstructBuildingAction.h"
#include <queue>
#include <time.h>
#include "TrainUnitAction.h"
#include "WorldManager.h"

class UnitManager
{
public:
	UnitManager(void);
	~UnitManager(void);
	void evalute(ActionQueue* actionQueue);
};
