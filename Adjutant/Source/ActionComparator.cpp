#include "ActionComparator.h"

ActionComparator::ActionComparator(void)
{
}

bool ActionComparator::operator() (Action* action1, Action* action2)
{
	//Higher priority has lower number
	if (action1->priority <= action2->priority)
	{
		return false;
	}
	else
	{
		return true;
	}
}

ActionComparator::~ActionComparator(void)
{
}
