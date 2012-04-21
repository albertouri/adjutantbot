#pragma once
#include "ActionQueue.h"

#include <BWAPI.h>
#include "MoveAction.h"
#include <queue>
#include "Utils.h"
#include "WorldModel.h"

class AwarenessModule
{
public:
	AwarenessModule(void);
	~AwarenessModule(void);
	void evalute(WorldModel* worldModel, ActionQueue* actionQueue);
};
