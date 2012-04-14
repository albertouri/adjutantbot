#pragma once
#include "Action.h"
class ActionComparator
{
public:
	ActionComparator(void);
	bool operator() (Action* action1, Action* action2);
	~ActionComparator(void);
};
