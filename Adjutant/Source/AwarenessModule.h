#pragma once
#include "Action.h"
#include "ActionComparator.h"
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
	void evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue);
};
