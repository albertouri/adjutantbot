#pragma once
#include <queue>
#include <BWAPI.h>
#include "Action.h"
#include "ActionComparator.h"
#include "ConstructBuildingAction.h"
#include "WorldManager.h"

class BuildManager
{
public:
	BuildManager(void);
	~BuildManager(void);
	static void evalute(ActionQueue* actionQueue);
};
