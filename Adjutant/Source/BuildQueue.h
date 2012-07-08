#pragma once
#include "BuildTaskComparator.h"
#include <queue>

class BuildQueue
{
public:
	BuildQueue(void);
	~BuildQueue(void);
	void push(BuildTask* t);
	BuildTask* BuildQueue::removeTop();
	int getScheduledCount(BWAPI::UnitType);
	std::priority_queue<BuildTask*, std::vector<BuildTask*>, BuildTaskComparator>* getPriorityQueue();

private:
	std::priority_queue<BuildTask*, std::vector<BuildTask*>, BuildTaskComparator>* queue;
	std::map<BWAPI::UnitType, int> scheduledUnits;
};
