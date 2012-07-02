#pragma once
#include "ActionQueue.h"

#include <BWAPI.h>
#include "MoveAction.h"
#include <queue>
#include "Utils.h"
#include "WorldManager.h"

class ScoutingManager
{
public:
	ScoutingManager(void);
	~ScoutingManager(void);
	void evalute(ActionQueue* actionQueue);
};
