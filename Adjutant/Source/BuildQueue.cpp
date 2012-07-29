#include "BuildQueue.h"

BuildQueue::BuildQueue(void)
{
	this->queue = new std::priority_queue<BuildTask*, std::vector<BuildTask*>, BuildTaskComparator>();
	this->scheduledUnits = std::map<BWAPI::UnitType, int>();
	this->scheduledTech = std::map<BWAPI::TechType, int>();
	this->scheduledUpgrades = std::map<BWAPI::UpgradeType, int>();
}

void BuildQueue::push(BuildTask* t)
{
	this->queue->push(t);
	if (t->isConstructBuilding() || t->isTrainUnit())
	{
		this->scheduledUnits[t->unitType]++;
	}
	else if (t->isTech())
	{
		this->scheduledTech[t->techType]++;
	}
	else if (t->isUpgrade())
	{
		this->scheduledUpgrades[t->upgradeType]++;
	}
}

BuildTask* BuildQueue::removeTop()
{
	BuildTask* t = this->queue->top();
	this->queue->pop();

	if (t->isConstructBuilding() || t->isTrainUnit())
	{
		this->scheduledUnits[t->unitType]--;
	}
	else if (t->isTech())
	{
		this->scheduledTech[t->techType]--;
	}
	else if (t->isUpgrade())
	{
		this->scheduledUpgrades[t->upgradeType]--;
	}

	return t;
}

int BuildQueue::getScheduledCount(BWAPI::UnitType type)
{
	return this->scheduledUnits[type];
}

int BuildQueue::getScheduledCount(BWAPI::TechType tech)
{
	return this->scheduledTech[tech];
}

int BuildQueue::getScheduledCount(BWAPI::UpgradeType upgrade)
{
	return this->scheduledUpgrades[upgrade];
}

std::priority_queue<BuildTask*, std::vector<BuildTask*>, BuildTaskComparator>* BuildQueue::getPriorityQueue()
{
	return this->queue;
}

BuildQueue::~BuildQueue(void)
{
	delete this->queue;
}
