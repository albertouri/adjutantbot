#pragma once
#include <queue>
#include <BWAPI.h>
#include "Action.h"
#include "ActionComparator.h"
#include "WorldManager.h"

class InformationManager
{
public:
	InformationManager(void);
	~InformationManager(void);
	static void evalute(ActionQueue* actionQueue);
};