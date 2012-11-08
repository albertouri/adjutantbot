#pragma once
#include "UCTGroup.h"
#include "UCTAction.h"
#include <vector>


class UCTNode
{
public:
	UCTNode(void);
	~UCTNode(void);
	bool equals(const UCTNode& node);

	std::vector<UCTAction*> possibleActions;
	std::vector<UCTGroup*> myGroups;
	std::vector<UCTGroup*> enemyGroups;
	std::map<UCTGroup*, UCTAction*> groupActionMap;

	UCTNode* parent;
};
