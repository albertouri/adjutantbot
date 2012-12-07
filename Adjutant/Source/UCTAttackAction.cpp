#include "UCTAttackAction.h"

UCTAttackAction::UCTAttackAction(int myGroupIndex, int enemyGroupIndex)
{
	this->type = AttackAction;
	this->myGroupIndex = myGroupIndex;
	this->enemyGroupIndex = enemyGroupIndex;

	std::stringstream stream;
	stream << myGroupIndex << " -> " << enemyGroupIndex;
	this->name = stream.str();
}

UCTAttackAction::~UCTAttackAction(void)
{
}
