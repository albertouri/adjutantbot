#include "ActionQueue.h"

ActionQueue::ActionQueue(void)
{
	actionVector = std::vector<Action*>();
}

std::vector<Action*>* ActionQueue::getActionVector()
{
	return &this->actionVector;
}

int ActionQueue::countBuildingActions(BWAPI::UnitType unitType)
{
	int count = 0;

	for each (Action* a in this->actionVector)
	{
		if (typeid(*a) == typeid(ConstructBuildingAction))
		{
			ConstructBuildingAction* action = (ConstructBuildingAction*)a;
			
			if (action->buildingType.getID() == unitType.getID())
			{
				count++;
			}
		}
	}

	return count;
}

void ActionQueue::push(Action* action)
{
	this->push(action, false);
}

void ActionQueue::push(Action* action, bool isForceAdd)
{
	bool isDuplicate = false;

	if (! isForceAdd)
	{
		//Instead of using a set, manually check for duplicates so that we can
		//support duplicate actions
		for each (Action* otherAction in this->actionVector)
		{
			if (*action == *otherAction)
			{
				isDuplicate = true;
				break;
			}
		}
	}

	if (! isDuplicate)
	{
		this->actionVector.push_back(action);
	}
}

void ActionQueue::clear()
{
	this->actionVector.clear();
}

bool ActionQueue::empty()
{
	return (this->actionVector.size() == 0);
}

std::priority_queue<Action*, std::vector<Action*>, ActionComparator> ActionQueue::getPrioritizedQueue()
{
	std::priority_queue<Action*, std::vector<Action*>, ActionComparator> actionQueue = std::priority_queue<Action*, std::vector<Action*>, ActionComparator>();

	for each(Action* action in this->actionVector)
	{
		actionQueue.push(action);
	}

	return actionQueue;
}

ActionQueue::~ActionQueue(void)
{
}
