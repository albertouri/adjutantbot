#pragma once
#include "BWAPI.h"
#include "UCTAttackAction.h"
#include "UCTGameState.h"
#include "UCTGroup.h"
#include "UCTJoinAction.h"
#include "UCTNode.h"
#include "WorldManager.h"

class UCTManager
{
public:
	UCTManager(void);
	~UCTManager(void);

	void evaluate();
private:
	static const int UCT_TOTAL_RUNS = 5000;
	static const int UCT_PER_ACTION_TRIES = 10;

	std::set<UCTNode*> allNodes;
	UCTNode* rootRoundNode;
	UCTNode* previousNode;
	UCTNode* nextNode;
	bool roundInProgress;
	int maxFriendlyGroups;
	int maxEnemyGroups;
	std::vector<UCTAction*> actionsTaken;
	std::vector<UCTNode*> nodesVisited;

	UCTGameState* getCurrentGameState();
	void formMyGroups();
	
	void onRoundStart();
	void onRoundEnd();
	void onDecisionPoint(UCTNode* node);
	void uctRun(UCTNode* currentNode, UCTGameState* gameState, bool isPolicyRun);
};
