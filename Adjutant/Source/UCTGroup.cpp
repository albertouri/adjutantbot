#include "UCTGroup.h"

UCTGroup::UCTGroup(void)
{
	this->currentAction = NULL;
}

UCTGroup::UCTGroup(int groupId, UnitGroup* group)
{
	this->currentAction = NULL;
	this->groupId = groupId;

	for each(BWAPI::Unit* unit in *(group->unitVector))
	{
		this->unitTypeMap[unit->getType()]++;
	}

	BWAPI::Position groupCenter =  group->getCentroid();
	this->effectiveHitPoints = group->getEffectiveHealth();
	this->positionX = groupCenter.x();
	this->positionY = groupCenter.y();
}

UCTGroup::UCTGroup(int groupId, Threat* threat)
{
	this->currentAction = NULL;
	this->groupId = groupId;

	for each(BWAPI::Unit* unit in threat->getUnits())
	{
		this->unitTypeMap[unit->getType()]++;
	}

	BWAPI::Position groupCenter =  threat->getCentroid();
	this->effectiveHitPoints = threat->getEffectiveHealth();
	this->positionX = groupCenter.x();
	this->positionY = groupCenter.y();
}

UCTGroup::~UCTGroup(void)
{
}
