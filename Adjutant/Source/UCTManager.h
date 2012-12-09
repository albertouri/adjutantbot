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
	enum HeuristicType
	{
		None,
		Closest,
		Random,
		Isolated
	}; 

	static const int UCT_TOTAL_RUNS = 1000;
	static const int UCT_PER_ACTION_TRIES = 10;
	static HeuristicType heuristicOnly;

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
	void executeActions(std::vector<UCTAction*>* actionVector);
	void uctRun(UCTNode* currentNode, UCTGameState* gameState, bool isPolicyRun);
	UCTAction* getMaxQValue(UCTNode* currentNode, std::vector<UCTAction*>* actionVector, bool isExploitOnly);
	void getTreeStringBFS(UCTNode* node, std::vector<std::string>* stream);

	//Heuristics only
	void heuristicClosest();
	void heuristicRandom();
	void heuristicIsolated();
};
