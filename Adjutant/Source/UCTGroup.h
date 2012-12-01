#pragma once
#include "BWAPI.h"
#include "Threat.h"
#include "UCTAction.h"
#include "UnitGroup.h"

class UCTGroup
{
public:
	UCTGroup(void);
	UCTGroup(int groupId, UnitGroup* group);
	UCTGroup(int groupId, Threat* group);
	~UCTGroup(void);
	
	std::map<BWAPI::UnitType, int> unitTypeMap;
	UCTAction *currentAction;
	float effectiveHitPoints;
	float friendlyDistanceTotal;
	float enemyDistanceTotal;
	int positionX;
	int positionY;
	int groupId;
};
