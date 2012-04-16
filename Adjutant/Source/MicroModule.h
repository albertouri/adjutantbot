#pragma once
#include "Action.h"
#include "AttackAction.h"
#include "ActionComparator.h"
#include <BWAPI.h>
#include <queue>
#include "Utils.h"
#include "WorldModel.h"

class MicroModule
{
public:
	MicroModule(void);
	~MicroModule(void);
	void evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue);
};
