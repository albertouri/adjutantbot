#pragma once
#include "UCTAttackAction.h"
#include "UCTJoinAction.h"
#include <vector>


class UCTNode
{
public:
	UCTNode(int maxFriendlyGroups, int maxEnemyGroups);
	~UCTNode(void);

	std::vector<UCTAction*> possibleActions;
	int visitCount;

private:
	void populatePossibleActions(int myGroupCount, int enemyGroupCount);
};
