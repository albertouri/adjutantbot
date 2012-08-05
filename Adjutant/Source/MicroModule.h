#pragma once
#include "ActionQueue.h"
#include "AttackAction.h"
#include <BWAPI.h>
#include <queue>
#include "Utils.h"
#include "WorldModel.h"

class MicroModule
{
public:
	MicroModule(void);
	~MicroModule(void);
	void evalute(WorldModel* worldModel, ActionQueue* actionQueue);
};
