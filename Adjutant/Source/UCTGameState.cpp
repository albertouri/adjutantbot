#include "UCTGameState.h"

float UCTGameState::SIM_ATTACK_ADVANTAGE_LOW_LIMIT = 0.5;
float UCTGameState::SIM_ATTACK_ADVANTAGE_HIGH_LIMIT = 5.0;

UCTGameState::UCTGameState(void)
{
}

bool UCTGameState::isLeaf()
{
	return (this->myGroups.size() == 0) | (this->enemyGroups.size() == 0);
}

bool UCTGameState::isValidAction(UCTAction* action)
{
	bool isValid = true;
	std::vector<int> friendlyGroupsNeeded = std::vector<int>();
	std::vector<int> enemyGroupsNeeded = std::vector<int>();

	if (action->type == UCTAction::AttackAction)
	{
		UCTAttackAction* attackAction = (UCTAttackAction*)action;
		friendlyGroupsNeeded.push_back(attackAction->myGroupIndex);
		enemyGroupsNeeded.push_back(attackAction->enemyGroupIndex);
	}
	else if (action->type == UCTAction::JoinAction)
	{
		UCTJoinAction* joinAction = (UCTJoinAction*)action;
		friendlyGroupsNeeded = joinAction->groupIdVector;
	}

	for each (int groupId in friendlyGroupsNeeded)
	{
		//Check if exists
		UCTGroup* existingGroup = NULL;

		for each (UCTGroup* testGroup in this->myGroups)
		{
			if (groupId == testGroup->groupId)
			{
				existingGroup = testGroup;
				break;
			}
		}

		if (existingGroup == NULL)
		{
			isValid = false;
			break;
		}

		//Make sure group is not busy
		if (this->groupActionMap.find((existingGroup)) != this->groupActionMap.end())
		//if (Utils::mapContains(&this->groupActionMap, existingGroup))
		{
			isValid = false;
			break;
		}
	}

	//Make sure enemy groups exist
	if (isValid && enemyGroupsNeeded.size() > 0)
	{
		for each (int groupId in enemyGroupsNeeded)
		{
			//Check if exists
			bool groupExists = false;

			for each (UCTGroup* testGroup in this->enemyGroups)
			{
				if (groupId == testGroup->groupId)
				{
					groupExists = true;
					break;
				}
			}

			if (! groupExists)
			{
				isValid = false;
				break;
			}
		}
	}

	return isValid;
}

bool UCTGameState::willTriggerSimulation(UCTAction* action)
{
	int freeGroupCount = this->myGroups.size() - this->groupActionMap.size();
	int willBusyCount = 0;

	//We assume the action is valid
	if (action->type == UCTAction::AttackAction)
	{
		UCTAttackAction* attackAction = (UCTAttackAction*)action;
		willBusyCount = 1;
	}
	else if (action->type == UCTAction::JoinAction)
	{
		UCTJoinAction* joinAction = (UCTJoinAction*)action;
		willBusyCount = joinAction->groupIdVector.size();
	}

	return ((freeGroupCount - willBusyCount) <= 0);
}

float UCTGameState::getRewardValue()
{
	//Calculate reward
	float reward = 0;

	for each (UCTGroup* myGroup in this->myGroups)
	{
		reward += myGroup->getEffectiveResourceValue();
	}

	for each (UCTGroup* enemyGroup in this->enemyGroups)
	{
		reward -= enemyGroup->getEffectiveResourceValue();
	}

	return reward;
}

void UCTGameState::markGroupsForAction(UCTAction* action)
{
	std::vector<int> busyGroupIds = std::vector<int>();

	//We assume the action is valid
	if (action->type == UCTAction::AttackAction)
	{
		UCTAttackAction* attackAction = (UCTAttackAction*)action;
		busyGroupIds.push_back(attackAction->myGroupIndex);
	}
	else if (action->type == UCTAction::JoinAction)
	{
		UCTJoinAction* joinAction = (UCTJoinAction*)action;
		busyGroupIds = joinAction->groupIdVector;
	}

	for each (UCTGroup* group in this->myGroups)
	{
		for each (int id in busyGroupIds)
		{
			if (group->groupId == id)
			{
				this->groupActionMap[group] = action;
			}
		}
	}
}

void UCTGameState::simulate()
{
	//this->enemyGroups.clear();	
	int freeGroupCount = 0;
	
	std::map<UCTGroup*, std::set<UCTGroup*>> attackingGroups;
	std::map<UCTJoinAction*, std::set<UCTGroup*>> joiningGroups;
	std::map<UCTGroup*, UCTGroup*> enemyAttackMap;

	//Simulate until we have a free group or one of the forces is defeated
	//Treat each group as point particles
	while(freeGroupCount == 0 && ! this->isLeaf())
	{
		//Generate enemy actions - assume they will attack my closest group
		for each (UCTGroup* enemyGroup in this->enemyGroups)
		{
			enemyAttackMap[enemyGroup] = this->getClosestGroup(enemyGroup, &this->myGroups);
		}

		//Simulate each friendly action
		for each (std::pair<UCTGroup*, UCTAction*> pair in this->groupActionMap)
		{
			UCTGroup* group = pair.first;
			UCTAction* action = pair.second;

			if (action->type == UCTAction::AttackAction)
			{
				UCTAttackAction* attackAction = (UCTAttackAction*)action;
				UCTGroup* targetGroup = Utils::getGroupById(&this->enemyGroups, attackAction->enemyGroupIndex);
				group->moveTowards(targetGroup, UCTGameState::SIM_FRAMES_PER_STEP);
			}
			else if (action->type == UCTAction::JoinAction)
			{
				UCTJoinAction* joinAction = (UCTJoinAction*)action;
				int x,y;
				joinAction->getCentroid(&this->myGroups, &x, &y);
				group->moveTowards(x, y, UCTGameState::SIM_FRAMES_PER_STEP);
			}
		}

		//Simulate each enemy action
		for each (std::pair<UCTGroup*, UCTGroup*> pair in enemyAttackMap)
		{
			UCTGroup* group = pair.first;
			group->moveTowards(enemyAttackMap[group], UCTGameState::SIM_FRAMES_PER_STEP);
		}

		//Check to see if any groups are close enough to attack each other
		for each (std::pair<UCTGroup*, UCTAction*> pair in this->groupActionMap)
		{
			UCTGroup* group = pair.first;
			UCTAction* action = pair.second;

			//Check for fights
			for each(UCTGroup* enemyGroup in this->enemyGroups)
			{
				if (group->getDistance(enemyGroup) <= UCTGameState::SIM_ATTACK_DISTANCE)
				{
					attackingGroups[enemyGroup].insert(group);
				}
			}
		}

		//Resolve attacks
		if (attackingGroups.size() > 0)
		{
			this->simulateAttacks(&attackingGroups);
		}

		//Check to see if any groups are close enough to join each other
		//These loops are purposefully split up so that if any groups are
		//destroyed from attacks, there are no dangling points
		for each (std::pair<UCTGroup*, UCTAction*> pair in this->groupActionMap)
		{
			UCTGroup* group = pair.first;
			UCTAction* action = pair.second;

			//Check for joins
			if (action->type == UCTAction::JoinAction)
			{
				UCTJoinAction* joinAction = (UCTJoinAction*)action;
				std::vector<UCTGroup*> groupVector = joinAction->getGroups(&this->myGroups);

				for each (UCTGroup* myOtherGroup in groupVector)
				{
					if (group != myOtherGroup && group->getDistance(myOtherGroup) < SIM_JOIN_DISTANCE)
					{
						joiningGroups[joinAction].insert(group);
						joiningGroups[joinAction].insert(myOtherGroup);
					}
				}
			}
		}

		//Resolve joins
		if (joiningGroups.size() > 0)
		{
			std::vector<UCTJoinAction*> actionsToRemove;

			for each (std::pair<UCTJoinAction*, std::set<UCTGroup*>> pair in joiningGroups)
			{
				UCTJoinAction* joinAction = pair.first;
				std::set<UCTGroup*> groups = pair.second;

				//If not all groups ready, don't join yet
				if (joinAction->groupIdVector.size() != groups.size())
				{
					actionsToRemove.push_back(joinAction);
				}
			}

			for each (UCTJoinAction* joinAction in actionsToRemove)
			{
				joiningGroups.erase(joinAction);
			}

			this->simulateJoins(&joiningGroups);
		}

		joiningGroups.clear();
		attackingGroups.clear();
		freeGroupCount = this->myGroups.size() - this->groupActionMap.size();
	}
}

void UCTGameState::simulateAttacks(std::map<UCTGroup*, std::set<UCTGroup*>>* attackingGroups)
{
	std::vector<UCTGroup*> groupsToDelete;

	for each (std::pair<UCTGroup*, std::set<UCTGroup*>> pair in (*attackingGroups))
	{
		UCTGroup* enemyGroup = pair.first;
		std::set<UCTGroup*> myGroupSet = pair.second;
		std::vector<HistoricalUnitInfo*> myCombinedUnits;
		std::map<HistoricalUnitInfo*, UCTGroup*> myCombinedMap;
		float groupAdvantage = 0;

		for each (UCTGroup* myGroup in myGroupSet)
		{
			if (! myGroup->unitVector.empty())
			{
				for each (HistoricalUnitInfo* unit in myGroup->unitVector)
				{
					myCombinedUnits.push_back(unit);
					myCombinedMap[unit] = myGroup;
				}

				groupAdvantage += myGroup->getEffectiveHealth();
			}
		}

		if (myCombinedUnits.empty() || enemyGroup->unitVector.empty()) {continue;}

		//Determine groupAdvantage factor based on who has the "bigger" army
		groupAdvantage /= enemyGroup->getEffectiveHealth();
		groupAdvantage = std::max(groupAdvantage, UCTGameState::SIM_ATTACK_ADVANTAGE_LOW_LIMIT);
		groupAdvantage = std::min(groupAdvantage, UCTGameState::SIM_ATTACK_ADVANTAGE_HIGH_LIMIT);

		//Pick 2 random units and fight them until one side loses
		while (enemyGroup->unitVector.size() > 0 && myCombinedUnits.size() > 0)
		{
			int myChoice = rand() % myCombinedUnits.size();
			int enemyChoice = rand() % enemyGroup->unitVector.size();

			HistoricalUnitInfo* myUnit = myCombinedUnits.at(myChoice);
			HistoricalUnitInfo* enemyUnit = enemyGroup->unitVector.at(enemyChoice);

			//TODO:Input learned type-type values
			//float damagePerHPLost = matrix[myUnit->getType()][enemyUnit->getType()];
			float damagePerHPLost = 1.0;
			damagePerHPLost *= groupAdvantage;

			int possibleDamage = (int)(((float)myUnit->getHitPoints()) * damagePerHPLost);

			if (possibleDamage > enemyUnit->getHitPoints())
			{
				//We win
				int myHitPointsLost = (int)(((float)enemyUnit->getHitPoints()) / damagePerHPLost);
				
				myUnit->setHitPoints(myUnit->getHitPoints() - myHitPointsLost);
				enemyUnit->setHitPoints(0);
			}
			else if (possibleDamage < enemyUnit->getHitPoints())
			{
				//We lose
				myUnit->setHitPoints(0);
				enemyUnit->setHitPoints(enemyUnit->getHitPoints() - possibleDamage);
			}
			else
			{
				//Draw
				myUnit->setHitPoints(0);
				enemyUnit->setHitPoints(0);
			}

			//Remove unit(s)
			if (myUnit->getHitPoints() <= 0)
			{
				Utils::vectorRemoveElement(&myCombinedUnits, myUnit);
				myCombinedMap[myUnit]->removeUnit(myUnit);
			}

			if (enemyUnit->getHitPoints() <= 0)
			{
				enemyGroup->removeUnit(enemyUnit);
			}
		}

		for each (UCTGroup* myGroup in myGroupSet)
		{
			if (myGroup->unitVector.empty())
			{
				UCTAction* action = this->groupActionMap[myGroup];
				Utils::vectorRemoveElement(&this->myGroups, myGroup);
				this->groupActionMap.erase(myGroup);

				//If was joining, set other involved groups to idle
				if (action != NULL && action->type == UCTAction::JoinAction)
				{
					UCTJoinAction* joinAction = (UCTJoinAction*)action;
					
					std::vector<UCTGroup*> joinGroups = joinAction->getGroups(&this->myGroups);

					for each(UCTGroup* joinGroup in joinGroups)
					{
						this->groupActionMap.erase(joinGroup);
					}
				}

				groupsToDelete.push_back(myGroup);
			}
		}

		if (enemyGroup->unitVector.empty())
		{
			Utils::vectorRemoveElement(&this->enemyGroups, enemyGroup);
			std::vector<UCTGroup*> groupsToRemove;

			for each (std::pair<UCTGroup*, UCTAction*> pair in this->groupActionMap)
			{
				UCTGroup* group = pair.first;
				UCTAction* action = pair.second;

				if (action->type == UCTAction::AttackAction)
				{
					UCTAttackAction* attackAction = (UCTAttackAction*)action;

					if (attackAction->enemyGroupIndex == enemyGroup->groupId)
					{
						groupsToRemove.push_back(group);
					}
				}
			}

			for each (UCTGroup* group in groupsToRemove)
			{
				this->groupActionMap.erase(group);
			}

			
			groupsToDelete.push_back(enemyGroup); 
		}
	}

	for each(UCTGroup* g in groupsToDelete)
	{
		delete g;
	}
}

void UCTGameState::simulateJoins(std::map<UCTJoinAction*, std::set<UCTGroup*>>* joiningGroups)
{
	for each (std::pair<UCTJoinAction*, std::set<UCTGroup*>> pair in (*joiningGroups))
	{
		UCTJoinAction* joinAction = pair.first;
		std::vector<UCTGroup*> groupVector;
		
		for each (UCTGroup* group in pair.second)
		{
			groupVector.push_back(group);
		}

		UCTGroup* mainGroup = groupVector.at(0);

		//Clear all mapped actions
		for each (UCTGroup* group in groupVector)
		{
			this->groupActionMap.erase(group);
		}

		//Merge groups
		for (unsigned int i=1; i<groupVector.size(); i++)
		{
			UCTGroup* otherGroup = groupVector.at(i);
			mainGroup->merge(otherGroup);
			Utils::vectorRemoveElement(&this->myGroups, otherGroup);
			delete otherGroup;
		}
	}
}

UCTGroup* UCTGameState::getClosestGroup(UCTGroup* group, std::vector<UCTGroup*>* otherGroups)
{
	UCTGroup* closestGroup = NULL;
	double minDist;

	for each (UCTGroup* otherGroup in (*otherGroups))
	{
		double newDist = otherGroup->getDistance(group);

		if (closestGroup == NULL || newDist < minDist)
		{
			minDist = newDist;
			closestGroup = otherGroup;
		}
	}

	return closestGroup;
}

UCTGameState::~UCTGameState(void)
{
	for each(UCTGroup* g in this->myGroups) {delete g;}
	for each(UCTGroup* g in this->enemyGroups) {delete g;}
	groupActionMap.clear();
}
