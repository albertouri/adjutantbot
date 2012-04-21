#pragma once
#include "Action.h"
#include "ActionComparator.h"
#include <queue>

class ActionQueue
{
public:
	ActionQueue(void);
	~ActionQueue(void);

	//Add a new action to the queue (will not be added if it is a duplicate)
	void push(Action* action);

	//Add a new action to the queue. Use second parameter to add action to queue even if it is a duplicate
	void push(Action* action, bool isForceAdd);

	//Clear all elements from the queue
	void clear();

	//Check if queue is empty
	bool empty();

	//Returns a priority queue of the underlying action vector]
	std::priority_queue<Action*, std::vector<Action*>, ActionComparator> getPrioritizedQueue();

private:
	std::vector<Action*> actionVector;
	
};
