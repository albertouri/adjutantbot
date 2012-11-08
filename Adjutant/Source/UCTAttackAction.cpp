#include "UCTAttackAction.h"

UCTAttackAction::UCTAttackAction(int myGroupIndex, int enemyGroupIndex)
{
	this->type = UCTActionType::AttackAction;
	this->myGroupIndex = myGroupIndex;
	this->enemyGroupIndex = enemyGroupIndex;
}

UCTAttackAction::~UCTAttackAction(void)
{
}
