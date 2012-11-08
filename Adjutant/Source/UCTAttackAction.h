#pragma once
#include "UCTAction.h"
#include "UCTGroup.h"

class UCTAttackAction : public UCTAction
{
public:
	UCTAttackAction(int myGroupIndex, int enemyGroupIndex);
	~UCTAttackAction(void);

	int myGroupIndex;
	int enemyGroupIndex;
};
