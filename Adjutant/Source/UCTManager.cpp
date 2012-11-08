#include "UCTManager.h"

UCTManager::UCTManager(void)
{
	this->root = NULL;
	this->roundInProgress = false;
}

void UCTManager::evaluate()
{
	int myUnitCount = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int enemyUnitCount = 0;

	for each (BWAPI::Player* enemy in BWAPI::Broodwar->enemies())
	{
		enemyUnitCount += enemy->completedUnitCount(BWAPI::UnitTypes::Terran_Marine);
	}

	//Check for round beginning and end
	if (this->roundInProgress)
	{
		//Round just ended
		if (myUnitCount == 0 || enemyUnitCount == 0)
		{
			this->onRoundEnd();
			this->roundInProgress = false;
		}
	}
	else
	{
		//Round just started
		if (myUnitCount > 0 && enemyUnitCount > 0)
		{
			this->onRoundStart();
			this->roundInProgress = true;
		}
	}
}

void UCTManager::onRoundStart()
{
	this->actionsTaken.clear();
	BWAPI::Broodwar->printf("Entering - OnRoundStart()");
	//Form groups
	const int MAX_CLUSTER_DISTANCE = 100;
	std::map<BWAPI::Unit*, UnitGroup*> unitGroupMap;
	std::set<BWAPI::Unit*> myMen = std::set<BWAPI::Unit*>();
	std::vector<UnitGroup*>* unitGroupVector = new std::vector<UnitGroup*>();
	UCTNode* currentNode;

	for each (BWAPI::Unit* unit in BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().canMove())
		{
			myMen.insert(unit);
		}
	}

	//TODO: Could be much more effecient
	//Determine threat groups using simple agglomerative hiearchial clustering
	for each (BWAPI::Unit* unit in myMen)
	{
		unitGroupMap[unit] = NULL;
	}

	for each (BWAPI::Unit* unit in myMen)
	{
		if (unitGroupMap[unit] != NULL) {continue;}

		for each (BWAPI::Unit* otherUnit in myMen)
		{
			if (unit == otherUnit) {continue;}

			if (unit->getDistance(otherUnit) < MAX_CLUSTER_DISTANCE)
			{
				if (unitGroupMap[otherUnit] != NULL)
				{
					unitGroupMap[unit] = unitGroupMap[otherUnit];
					unitGroupMap[otherUnit]->addUnit(unit);
				}
				else
				{
					UnitGroup* group = new UnitGroup();
					group->addUnit(unit);
					group->addUnit(otherUnit);

					unitGroupMap[unit] = group;
					unitGroupMap[otherUnit] = group;
					unitGroupVector->push_back(group);
				}

				break;
			}
		}

		if (unitGroupMap[unit] == NULL)
		{
			UnitGroup* group = new UnitGroup();
			group->addUnit(unit);
			unitGroupVector->push_back(group);
		}
	}

	WorldManager::Instance().myArmyGroups = unitGroupVector;
	WorldManager::Instance().groupAttackMap.clear();

	if (this->root != NULL)
	{
		currentNode = this->root;
	}
	else
	{
		currentNode = getCurrentNode();
		this->root = currentNode;
	}

	std::vector<UCTAction*> actionsToExecute = std::vector<UCTAction*>();
	std::vector<UCTAction*> actionsToRemove = std::vector<UCTAction*>();
	std::vector<UCTAction*> remainingActions = currentNode->possibleActions;
	std::vector<int> groupsMadeBusy = std::vector<int>();

	while (! remainingActions.empty())
	{
		groupsMadeBusy.clear();
		actionsToRemove.clear();
		int choice = rand() % remainingActions.size();
		UCTAction* currentAction = remainingActions.at(choice);

		if (currentAction->type == UCTAction::AttackAction)
		{
			UCTAttackAction* attackAction = (UCTAttackAction*)currentAction;
			groupsMadeBusy.push_back(attackAction->myGroupIndex);
		}

		for each(UCTAction* action in remainingActions)
		{
			for each (int groupId in groupsMadeBusy)
			{
				if (action->type == UCTAction::AttackAction)
				{
					UCTAttackAction* attackAction = (UCTAttackAction*)action;
					
					if (attackAction->myGroupIndex == groupId)
					{
						actionsToRemove.push_back(action);
						break;
					}
				}
			}
		}

		for each (UCTAction* action in actionsToRemove)
		{
			Utils::vectorRemoveElement(&remainingActions, action);
		}

		actionsToExecute.push_back(currentAction);
	}

	for each(UCTAction* action in actionsToExecute)
	{
		if (action->type == UCTAction::AttackAction)
		{
			UCTAttackAction* attackAction = (UCTAttackAction*)action;
			UnitGroup* myGroup = WorldManager::Instance().myArmyGroups->at(attackAction->myGroupIndex);
			Threat* threatToAttack = WorldManager::Instance().threatVector.at(attackAction->enemyGroupIndex);

			WorldManager::Instance().groupAttackMap[myGroup] = threatToAttack;

			this->actionsTaken.push_back(action);
			action->visitCount++;
		}
	}

	/*
	for each(UnitGroup* group in *unitGroupVector)
	{
		int choice = rand() % WorldManager::Instance().threatVector.size(); //randomly choose a threat
		WorldManager::Instance().groupAttackMap[group] = WorldManager::Instance().threatVector.at(choice);
	}
	*/
}

void UCTManager::onRoundEnd()
{
	//Calculate reward
	float reward = 0;

	for each (UnitGroup* unitGroup in *(WorldManager::Instance().myArmyGroups))
	{
		reward += unitGroup->getEffectiveHealth();
	}

	for each (Threat* threat in WorldManager::Instance().threatVector)
	{
		reward -= threat->getEffectiveHealth();
	}

	for each (UCTAction* action in this->actionsTaken)
	{
		BWAPI::Broodwar->printf("Reward %f", reward); 
		action->totalReward += reward;
	}

	//Temp
	for each (UCTAction* action in this->root->possibleActions)
	{
		if (action->type == UCTAction::AttackAction)
		{
			UCTAttackAction* attackAction = (UCTAttackAction*)action;

			double averageReward = 0;

			if (attackAction->visitCount > 0)
			{
				averageReward = attackAction->totalReward / attackAction->visitCount;
			}

			BWAPI::Broodwar->printf("Attack %d->%d [v:%d][avg_r:%f]", 
				attackAction->myGroupIndex, 
				attackAction->enemyGroupIndex,
				attackAction->visitCount,
				averageReward);
		}
	}
}

UCTNode* UCTManager::getCurrentNode()
{
	int count = 0;
	bool isPreviousNode = false;
	UCTNode* newNode = new UCTNode();

	for each (UnitGroup* group in *(WorldManager::Instance().myArmyGroups))
	{
		newNode->myGroups.push_back(new UCTGroup(count, group));
		count++;
	}

	count = 0;
	for each (Threat* threat in WorldManager::Instance().threatVector)
	{
		newNode->enemyGroups.push_back(new UCTGroup(count, threat));
		count++;
	}

	//Determine if we have seen this node before
	for each (UCTNode* node in this->allNodes)
	{
		if (node->equals(*newNode))
		{
			delete newNode;
			newNode = node;
			isPreviousNode = true;
			break;
		}
	}

	//Generate all possible actions for this state
	if (! isPreviousNode)
	{
		populatePossibleActions(newNode);
	}

	return newNode;
}

void UCTManager::populatePossibleActions(UCTNode* node)
{
	//Generate join actions
	//TODO

	//Generate attack actions
	for each (UCTGroup* myGroup in node->myGroups)
	{
		for each (UCTGroup* enemyGroup in node->enemyGroups)
		{
			node->possibleActions.push_back(
				new UCTAttackAction(myGroup->groupId, enemyGroup->groupId)
				);
		}
	}
}

UCTManager::~UCTManager(void)
{
}
