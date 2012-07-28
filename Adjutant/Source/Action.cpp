#include "Action.h"

//Generic action methods. Should be overwritten for each action
Action::Action(void)
{
}

void Action::updateResourceCost(int* minerals, int* gas, int* supplyRemaining)
{
	//do nothing - action has no resource cost	
}

bool Action::operator!=(const Action &other) const {
	return !(*this == other);
}

Action::~Action(void)
{
}
