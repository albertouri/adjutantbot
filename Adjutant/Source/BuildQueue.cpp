#include "BuildQueue.h"

BuildQueue::BuildQueue(void)
{
	this->queue = new std::priority_queue<BuildTask*, std::vector<BuildTask*>, BuildTaskComparator>();
	this->scheduledUnits = std::map<BWAPI::UnitType, int>();
}

void BuildQueue::push(BuildTask* t)
{
	this->queue->push(t);
	if (t->isConstructBuilding() || t->isTrainUnit())
	{
		this->scheduledUnits[t->unitType]++;
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

	return t;
}

int BuildQueue::getScheduledCount(BWAPI::UnitType type)
{
	return this->scheduledUnits[type];
}

std::priority_queue<BuildTask*, std::vector<BuildTask*>, BuildTaskComparator>* BuildQueue::getPriorityQueue()
{
	return this->queue;
}

BuildQueue::~BuildQueue(void)
{
	delete this->queue;
}
