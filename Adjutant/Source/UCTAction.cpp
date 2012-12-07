#include "UCTAction.h"
#include "UCTNode.h"

UCTAction::UCTAction(void)
{
	this->name = "";
	this->resultantNode = NULL;
	this->totalReward = 0;
	this->visitCount = 0;
	this->type = Invalid;
}

UCTAction::~UCTAction(void)
{
}
