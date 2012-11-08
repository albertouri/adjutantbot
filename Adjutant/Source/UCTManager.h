#pragma once
#include "BWAPI.h"
#include "UCTAttackAction.h"
#include "UCTNode.h"
#include "WorldManager.h"

class UCTManager
{
public:
	UCTManager(void);
	~UCTManager(void);

	void evaluate();

private:
	bool roundInProgress;
	UCTNode* root;
	std::set<UCTNode*> allNodes;
	std::vector<UCTAction*> actionsTaken;

	UCTNode* getCurrentNode();
	void populatePossibleActions(UCTNode* node);

	void onRoundStart();
	void onRoundEnd();
};
