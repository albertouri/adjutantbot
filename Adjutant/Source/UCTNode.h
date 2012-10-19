#pragma once
#include "UnitGroup.h"
#include <vector>

class UCTNode
{
public:
	UCTNode(void);
	~UCTNode(void);

	std::vector<UnitGroup*> myGroups;
	std::vector<UnitGroup*> enemyGroups;
	UCTNode* parent;
};
