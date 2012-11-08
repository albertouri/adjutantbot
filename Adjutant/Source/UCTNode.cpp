#include "UCTNode.h"

UCTNode::UCTNode(void)
{
}

bool UCTNode::equals(const UCTNode &node)
{
	bool isEqual = false;

	return isEqual;
}

UCTNode::~UCTNode(void)
{
	for each(UCTGroup* g in this->myGroups) {delete g;}
	for each(UCTGroup* g in this->enemyGroups) {delete g;}
	for each(UCTAction* a in this->possibleActions) {delete a;}
}
