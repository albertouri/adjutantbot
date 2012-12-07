#include "UCTNode.h"

UCTNode::UCTNode(int maxFriendlyGroups, int maxEnemyGroups)
{
	this->visitCount = 0;

	this->populatePossibleActions(maxFriendlyGroups, maxEnemyGroups);
}

void UCTNode::populatePossibleActions(int myGroupCount, int enemyGroupCount)
{
	//Add join actions
	//TODO:We could generate the unique join combinations... hardcoded for now
	if (myGroupCount >= 2)
	{
		this->possibleActions.push_back(new UCTJoinAction(0, 1));
	} 

	if (myGroupCount >= 3)
	{
		this->possibleActions.push_back(new UCTJoinAction(1, 2));
		this->possibleActions.push_back(new UCTJoinAction(0, 2));
		this->possibleActions.push_back(new UCTJoinAction(0, 1, 2));
	}
	
	if (myGroupCount >= 4)
	{
		this->possibleActions.push_back(new UCTJoinAction(2, 3));
		this->possibleActions.push_back(new UCTJoinAction(1, 3));
		this->possibleActions.push_back(new UCTJoinAction(1, 2, 3));
		this->possibleActions.push_back(new UCTJoinAction(0, 3));
		this->possibleActions.push_back(new UCTJoinAction(0, 2, 3));
		this->possibleActions.push_back(new UCTJoinAction(0, 1, 3));
		this->possibleActions.push_back(new UCTJoinAction(0, 1, 2, 3));
	}

	if (myGroupCount > 4)
	{
		Utils::log("WARNING: Join actions for group sizes > 4 not supported)", 0);
	}

	//Generate attack actions
	for (int i=0; i<myGroupCount; i++)
	{
		for (int j=0; j<myGroupCount; j++)
		{
			this->possibleActions.push_back(new UCTAttackAction(i, j));
		}
	}
}

UCTNode::~UCTNode(void)
{
	for each(UCTAction* a in this->possibleActions) {delete a;}
}
