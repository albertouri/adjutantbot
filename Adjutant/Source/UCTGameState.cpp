#include "UCTGameState.h"

float UCTGameState::SIM_ATTACK_ADVANTAGE_LOW_LIMIT = 0.5;
float UCTGameState::SIM_ATTACK_ADVANTAGE_HIGH_LIMIT = 5.0;
float** UCTGameState::matrix = NULL;
UCTGameState::UCTGameState(void)
{
	if (matrix == NULL)
	{
		this->initTrainingMatrix();
	}
}

bool UCTGameState::isLeaf()
{
	return (this->myGroups.size() == 0) | (this->enemyGroups.size() == 0);
}

bool UCTGameState::isValidAction(UCTAction* action)
{
	Utils::log("Entering UCTGameState - isValidAction", 6);
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

	Utils::log("Leaving UCTGameState - isValidAction", 6);
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
	Utils::log("Entering UCTManager - simulate", 4);
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

	Utils::log("Leaving UCTManager - simulate", 4);
}

void UCTGameState::simulateAttacks(std::map<UCTGroup*, std::set<UCTGroup*>>* attackingGroups)
{
	Utils::log("Entering UCTManager - simulateAttacks", 5);
	std::set<UCTGroup*> groupsToDelete;

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
			float damagePerHPLost = matrix[myUnit->getType()][enemyUnit->getType()];
			//float damagePerHPLost = 1.0;
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

				groupsToDelete.insert(myGroup);
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

			
			groupsToDelete.insert(enemyGroup); 
		}
	}

	for each(UCTGroup* g in groupsToDelete)
	{
		delete g;
	}

	Utils::log("Leaving UCTManager - simulateAttacks", 5);
}

void UCTGameState::simulateJoins(std::map<UCTJoinAction*, std::set<UCTGroup*>>* joiningGroups)
{
	Utils::log("Entering UCTManager - simulateJoins", 5);
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
	Utils::log("Leaving UCTManager - simulateJoins", 5);
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

void UCTGameState::initTrainingMatrix()
{
	int allUnitsSize = BWAPI::UnitTypes::allUnitTypes().size();
	this->matrix = new float*[allUnitsSize];
	for (int i=0; i<allUnitsSize; i++)
	{
		matrix[i] = new float[allUnitsSize];
	}

	matrix[0][0] = 0.83f;			// Terran Marine vs. Terran Marine
	matrix[0][1] = 0.60f;			// Terran Marine vs. Terran Ghost
	matrix[0][2] = 0.33f;			// Terran Marine vs. Terran Vulture
	matrix[0][3] = 0.54f;			// Terran Marine vs. Terran Goliath
	matrix[0][5] = 0.52f;			// Terran Marine vs. Terran Siege Tank
	matrix[0][7] = 1.03f;			// Terran Marine vs. Terran SCV
	matrix[0][8] = 1.10f;			// Terran Marine vs. Terran Wraith
	matrix[0][9] = 200.00f;		// Terran Marine vs. Terran Science Vessel
	matrix[0][11] = 150.00f;		// Terran Marine vs. Terran Dropship
	matrix[0][12] = 0.19f;		// Terran Marine vs. Terran Battlecruiser
	matrix[0][32] = 0.52f;		// Terran Marine vs. Terran f;irebat
	matrix[0][34] = 60.00f;		// Terran Marine vs. Terran Medic
	matrix[0][37] = 1.00f;		// Terran Marine vs. Zerg Zergling
	matrix[0][38] = 0.77f;		// Terran Marine vs. Zerg Hydralisk
	matrix[0][39] = 0.03f;		// Terran Marine vs. Zerg Ultralisk
	matrix[0][40] = 1.48f;		// Terran Marine vs. Zerg Broodling
	matrix[0][41] = 1.51f;		// Terran Marine vs. Zerg Drone
	matrix[0][42] = 191.17f;		// Terran Marine vs. Zerg Overlord
	matrix[0][43] = 0.89f;		// Terran Marine vs. Zerg Mutalisk
	matrix[0][45] = 120.00f;		// Terran Marine vs. Zerg Queen
	matrix[0][46] = 80.00f;		// Terran Marine vs. Zerg Def;iler
	matrix[0][47] = 25.00f;		// Terran Marine vs. Zerg Scourge
	matrix[0][50] = 1.50f;		// Terran Marine vs. Zerg Inf;ested Terran
	matrix[0][58] = 200.00f;		// Terran Marine vs. Terran Valkyrie
	matrix[0][60] = 100.00f;		// Terran Marine vs. Protoss Corsair
	matrix[0][61] = 1.00f;		// Terran Marine vs. Protoss Dark Templar
	matrix[0][62] = 137.83f;		// Terran Marine vs. Zerg Devourer
	matrix[0][63] = 25.00f;		// Terran Marine vs. Protoss Dark Archon
	matrix[0][64] = 0.69f;		// Terran Marine vs. Protoss Probe
	matrix[0][65] = 0.03f;		// Terran Marine vs. Protoss Zealot
	matrix[0][66] = 0.40f;		// Terran Marine vs. Protoss Dragoon
	matrix[0][67] = 40.00f;		// Terran Marine vs. Protoss High Templar
	matrix[0][68] = 0.03f;		// Terran Marine vs. Protoss Archon
	matrix[0][69] = 80.00f;		// Terran Marine vs. Protoss Shuttle
	matrix[0][70] = 0.03f;		// Terran Marine vs. Protoss Scout
	matrix[0][72] = 179.50f;		// Terran Marine vs. Protoss Carrier
	matrix[0][83] = 100.00f;		// Terran Marine vs. Protoss Reaver
	matrix[0][84] = 1.00f;		// Terran Marine vs. Protoss Observer
	matrix[0][103] = 2.42f;		// Terran Marine vs. Zerg Lurker
	matrix[1][0] = 1.00f;			// Terran Ghost vs. Terran Marine
	matrix[1][1] = 0.99f;			// Terran Ghost vs. Terran Ghost
	matrix[1][2] = 0.33f;			// Terran Ghost vs. Terran Vulture
	matrix[1][3] = 0.17f;			// Terran Ghost vs. Terran Goliath
	matrix[1][5] = 0.17f;			// Terran Ghost vs. Terran Siege Tank
	matrix[1][7] = 1.47f;			// Terran Ghost vs. Terran SCV
	matrix[1][8] = 0.37f;			// Terran Ghost vs. Terran Wraith
	matrix[1][9] = 200.00f;		// Terran Ghost vs. Terran Science Vessel
	matrix[1][11] = 150.00f;		// Terran Ghost vs. Terran Dropship
	matrix[1][12] = 0.06f;		// Terran Ghost vs. Terran Battlecruiser
	matrix[1][32] = 0.73f;		// Terran Ghost vs. Terran f;irebat
	matrix[1][34] = 60.00f;		// Terran Ghost vs. Terran Medic
	matrix[1][37] = 0.86f;		// Terran Ghost vs. Zerg Zergling
	matrix[1][38] = 0.24f;		// Terran Ghost vs. Zerg Hydralisk
	matrix[1][39] = 0.02f;		// Terran Ghost vs. Zerg Ultralisk
	matrix[1][40] = 2.02f;		// Terran Ghost vs. Zerg Broodling
	matrix[1][41] = 1.85f;		// Terran Ghost vs. Zerg Drone
	matrix[1][42] = 97.50f;		// Terran Ghost vs. Zerg Overlord
	matrix[1][43] = 0.74f;		// Terran Ghost vs. Zerg Mutalisk
	matrix[1][45] = 120.00f;		// Terran Ghost vs. Zerg Queen
	matrix[1][46] = 80.00f;		// Terran Ghost vs. Zerg Def;iler
	matrix[1][47] = 25.00f;		// Terran Ghost vs. Zerg Scourge
	matrix[1][50] = 1.33f;		// Terran Ghost vs. Zerg Inf;ested Terran
	matrix[1][58] = 170.00f;		// Terran Ghost vs. Terran Valkyrie
	matrix[1][60] = 100.00f;		// Terran Ghost vs. Protoss Corsair
	matrix[1][61] = 1.00f;		// Terran Ghost vs. Protoss Dark Templar
	matrix[1][62] = 230.00f;		// Terran Ghost vs. Zerg Devourer
	matrix[1][63] = 0.02f;		// Terran Ghost vs. Protoss Dark Archon
	matrix[1][64] = 0.83f;		// Terran Ghost vs. Protoss Probe
	matrix[1][65] = 0.02f;		// Terran Ghost vs. Protoss Zealot
	matrix[1][66] = 0.12f;		// Terran Ghost vs. Protoss Dragoon
	matrix[1][67] = 40.00f;		// Terran Ghost vs. Protoss High Templar
	matrix[1][68] = 0.02f;		// Terran Ghost vs. Protoss Archon
	matrix[1][69] = 80.00f;		// Terran Ghost vs. Protoss Shuttle
	matrix[1][70] = 0.02f;		// Terran Ghost vs. Protoss Scout
	matrix[1][72] = 121.50f;		// Terran Ghost vs. Protoss Carrier
	matrix[1][83] = 100.00f;		// Terran Ghost vs. Protoss Reaver
	matrix[1][84] = 1.00f;		// Terran Ghost vs. Protoss Observer
	matrix[1][103] = 2.60f;		// Terran Ghost vs. Zerg Lurker
	matrix[2][0] = 1.00f;			// Terran Vulture vs. Terran Marine
	matrix[2][1] = 2.75f;			// Terran Vulture vs. Terran Ghost
	matrix[2][2] = 0.94f;			// Terran Vulture vs. Terran Vulture
	matrix[2][3] = 0.29f;			// Terran Vulture vs. Terran Goliath
	matrix[2][5] = 0.28f;			// Terran Vulture vs. Terran Siege Tank
	matrix[2][7] = 3.30f;			// Terran Vulture vs. Terran SCV
	matrix[2][8] = 0.01f;			// Terran Vulture vs. Terran Wraith
	matrix[2][9] = 1.00f;			// Terran Vulture vs. Terran Science Vessel
	matrix[2][11] = 1.00f;		// Terran Vulture vs. Terran Dropship
	matrix[2][12] = 0.01f;		// Terran Vulture vs. Terran Battlecruiser
	matrix[2][32] = 1.78f;		// Terran Vulture vs. Terran f;irebat
	matrix[2][34] = 60.00f;		// Terran Vulture vs. Terran Medic
	matrix[2][37] = 1.80f;		// Terran Vulture vs. Zerg Zergling
	matrix[2][38] = 0.15f;		// Terran Vulture vs. Zerg Hydralisk
	matrix[2][39] = 0.01f;		// Terran Vulture vs. Zerg Ultralisk
	matrix[2][40] = 3.75f;		// Terran Vulture vs. Zerg Broodling
	matrix[2][41] = 2.22f;		// Terran Vulture vs. Zerg Drone
	matrix[2][42] = 1.00f;		// Terran Vulture vs. Zerg Overlord
	matrix[2][43] = 0.01f;		// Terran Vulture vs. Zerg Mutalisk
	matrix[2][45] = 1.00f;		// Terran Vulture vs. Zerg Queen
	matrix[2][46] = 80.00f;		// Terran Vulture vs. Zerg Def;iler
	matrix[2][47] = 1.00f;		// Terran Vulture vs. Zerg Scourge
	matrix[2][50] = 0.75f;		// Terran Vulture vs. Zerg Inf;ested Terran
	matrix[2][58] = 1.00f;		// Terran Vulture vs. Terran Valkyrie
	matrix[2][60] = 1.00f;		// Terran Vulture vs. Protoss Corsair
	matrix[2][61] = 1.00f;		// Terran Vulture vs. Protoss Dark Templar
	matrix[2][62] = 1.00f;		// Terran Vulture vs. Zerg Devourer
	matrix[2][63] = 25.00f;		// Terran Vulture vs. Protoss Dark Archon
	matrix[2][64] = 1.67f;		// Terran Vulture vs. Protoss Probe
	matrix[2][65] = 0.07f;		// Terran Vulture vs. Protoss Zealot
	matrix[2][66] = 0.19f;		// Terran Vulture vs. Protoss Dragoon
	matrix[2][67] = 40.00f;		// Terran Vulture vs. Protoss High Templar
	matrix[2][68] = 0.01f;		// Terran Vulture vs. Protoss Archon
	matrix[2][69] = 1.00f;		// Terran Vulture vs. Protoss Shuttle
	matrix[2][70] = 0.01f;		// Terran Vulture vs. Protoss Scout
	matrix[2][72] = 1.00f;		// Terran Vulture vs. Protoss Carrier
	matrix[2][83] = 100.00f;		// Terran Vulture vs. Protoss Reaver
	matrix[2][84] = 1.00f;		// Terran Vulture vs. Protoss Observer
	matrix[2][103] = 2.08f;		// Terran Vulture vs. Zerg Lurker
	matrix[3][0] = 1.60f;			// Terran Goliath vs. Terran Marine
	matrix[3][1] = 6.25f;			// Terran Goliath vs. Terran Ghost
	matrix[3][2] = 3.38f;			// Terran Goliath vs. Terran Vulture
	matrix[3][3] = 0.99f;			// Terran Goliath vs. Terran Goliath
	matrix[3][5] = 0.69f;			// Terran Goliath vs. Terran Siege Tank
	matrix[3][7] = 2.32f;			// Terran Goliath vs. Terran SCV
	matrix[3][8] = 3.72f;			// Terran Goliath vs. Terran Wraith
	matrix[3][9] = 200.00f;		// Terran Goliath vs. Terran Science Vessel
	matrix[3][11] = 150.00f;		// Terran Goliath vs. Terran Dropship
	matrix[3][12] = 1.42f;		// Terran Goliath vs. Terran Battlecruiser
	matrix[3][32] = 2.94f;		// Terran Goliath vs. Terran f;irebat
	matrix[3][34] = 60.00f;		// Terran Goliath vs. Terran Medic
	matrix[3][37] = 1.67f;		// Terran Goliath vs. Zerg Zergling
	matrix[3][38] = 0.99f;		// Terran Goliath vs. Zerg Hydralisk
	matrix[3][39] = 0.39f;		// Terran Goliath vs. Zerg Ultralisk
	matrix[3][40] = 3.33f;		// Terran Goliath vs. Zerg Broodling
	matrix[3][41] = 2.64f;		// Terran Goliath vs. Zerg Drone
	matrix[3][42] = 200.00f;		// Terran Goliath vs. Zerg Overlord
	matrix[3][43] = 1.50f;		// Terran Goliath vs. Zerg Mutalisk
	matrix[3][45] = 120.00f;		// Terran Goliath vs. Zerg Queen
	matrix[3][46] = 80.00f;		// Terran Goliath vs. Zerg Def;iler
	matrix[3][47] = 25.00f;		// Terran Goliath vs. Zerg Scourge
	matrix[3][50] = 0.48f;		// Terran Goliath vs. Zerg Inf;ested Terran
	matrix[3][58] = 200.00f;		// Terran Goliath vs. Terran Valkyrie
	matrix[3][60] = 100.00f;		// Terran Goliath vs. Protoss Corsair
	matrix[3][61] = 0.01f;		// Terran Goliath vs. Protoss Dark Templar
	matrix[3][62] = 250.00f;		// Terran Goliath vs. Zerg Devourer
	matrix[3][63] = 25.00f;		// Terran Goliath vs. Protoss Dark Archon
	matrix[3][64] = 1.39f;		// Terran Goliath vs. Protoss Probe
	matrix[3][65] = 0.25f;		// Terran Goliath vs. Protoss Zealot
	matrix[3][66] = 0.84f;		// Terran Goliath vs. Protoss Dragoon
	matrix[3][67] = 40.00f;		// Terran Goliath vs. Protoss High Templar
	matrix[3][68] = 0.01f;		// Terran Goliath vs. Protoss Archon
	matrix[3][69] = 80.00f;		// Terran Goliath vs. Protoss Shuttle
	matrix[3][70] = 2.26f;		// Terran Goliath vs. Protoss Scout
	matrix[3][72] = 300.00f;		// Terran Goliath vs. Protoss Carrier
	matrix[3][83] = 100.00f;		// Terran Goliath vs. Protoss Reaver
	matrix[3][84] = 1.00f;		// Terran Goliath vs. Protoss Observer
	matrix[3][103] = 4.34f;		// Terran Goliath vs. Zerg Lurker
	matrix[5][0] = 1.00f;			// Terran Siege Tank vs. Terran Marine
	matrix[5][1] = 5.00f;			// Terran Siege Tank vs. Terran Ghost
	matrix[5][2] = 3.99f;			// Terran Siege Tank vs. Terran Vulture
	matrix[5][3] = 1.59f;			// Terran Siege Tank vs. Terran Goliath
	matrix[5][5] = 1.03f;			// Terran Siege Tank vs. Terran Siege Tank
	matrix[5][7] = 1.88f;			// Terran Siege Tank vs. Terran SCV
	matrix[5][8] = 1.00f;			// Terran Siege Tank vs. Terran Wraith
	matrix[5][9] = 1.00f;			// Terran Siege Tank vs. Terran Science Vessel
	matrix[5][11] = 1.00f;		// Terran Siege Tank vs. Terran Dropship
	matrix[5][12] = 1.00f;		// Terran Siege Tank vs. Terran Battlecruiser
	matrix[5][32] = 2.61f;		// Terran Siege Tank vs. Terran f;irebat
	matrix[5][34] = 1.00f;		// Terran Siege Tank vs. Terran Medic
	matrix[5][37] = 0.93f;		// Terran Siege Tank vs. Zerg Zergling
	matrix[5][38] = 1.10f;		// Terran Siege Tank vs. Zerg Hydralisk
	matrix[5][39] = 0.49f;		// Terran Siege Tank vs. Zerg Ultralisk
	matrix[5][40] = 2.00f;		// Terran Siege Tank vs. Zerg Broodling
	matrix[5][41] = 2.50f;		// Terran Siege Tank vs. Zerg Drone
	matrix[5][42] = 1.00f;		// Terran Siege Tank vs. Zerg Overlord
	matrix[5][43] = 1.00f;		// Terran Siege Tank vs. Zerg Mutalisk
	matrix[5][45] = 1.00f;		// Terran Siege Tank vs. Zerg Queen
	matrix[5][46] = 80.00f;		// Terran Siege Tank vs. Zerg Def;iler
	matrix[5][47] = 1.00f;		// Terran Siege Tank vs. Zerg Scourge
	matrix[5][50] = 0.40f;		// Terran Siege Tank vs. Zerg Inf;ested Terran
	matrix[5][58] = 1.00f;		// Terran Siege Tank vs. Terran Valkyrie
	matrix[5][60] = 1.00f;		// Terran Siege Tank vs. Protoss Corsair
	matrix[5][61] = 1.00f;		// Terran Siege Tank vs. Protoss Dark Templar
	matrix[5][62] = 1.00f;		// Terran Siege Tank vs. Zerg Devourer
	matrix[5][63] = 25.00f;		// Terran Siege Tank vs. Protoss Dark Archon
	matrix[5][64] = 1.25f;		// Terran Siege Tank vs. Protoss Probe
	matrix[5][65] = 0.45f;		// Terran Siege Tank vs. Protoss Zealot
	matrix[5][66] = 2.10f;		// Terran Siege Tank vs. Protoss Dragoon
	matrix[5][67] = 40.00f;		// Terran Siege Tank vs. Protoss High Templar
	matrix[5][68] = 0.01f;		// Terran Siege Tank vs. Protoss Archon
	matrix[5][69] = 1.00f;		// Terran Siege Tank vs. Protoss Shuttle
	matrix[5][70] = 0.01f;		// Terran Siege Tank vs. Protoss Scout
	matrix[5][72] = 1.00f;		// Terran Siege Tank vs. Protoss Carrier
	matrix[5][83] = 100.00f;		// Terran Siege Tank vs. Protoss Reaver
	matrix[5][84] = 1.00f;		// Terran Siege Tank vs. Protoss Observer
	matrix[5][103] = 1.00f;		// Terran Siege Tank vs. Zerg Lurker
	matrix[8][0] = 0.72f;			// Terran Wraith vs. Terran Marine
	matrix[8][1] = 2.27f;			// Terran Wraith vs. Terran Ghost
	matrix[8][2] = 78.67f;		// Terran Wraith vs. Terran Vulture
	matrix[8][3] = 0.28f;			// Terran Wraith vs. Terran Goliath
	matrix[8][5] = 97.83f;		// Terran Wraith vs. Terran Siege Tank
	matrix[8][7] = 60.00f;		// Terran Wraith vs. Terran SCV
	matrix[8][8] = 0.98f;			// Terran Wraith vs. Terran Wraith
	matrix[8][9] = 133.89f;		// Terran Wraith vs. Terran Science Vessel
	matrix[8][11] = 75.63f;		// Terran Wraith vs. Terran Dropship
	matrix[8][12] = 0.85f;		// Terran Wraith vs. Terran Battlecruiser
	matrix[8][32] = 50.00f;		// Terran Wraith vs. Terran f;irebat
	matrix[8][34] = 60.00f;		// Terran Wraith vs. Terran Medic
	matrix[8][37] = 1.00f;		// Terran Wraith vs. Zerg Zergling
	matrix[8][38] = 0.24f;		// Terran Wraith vs. Zerg Hydralisk
	matrix[8][39] = 230.50f;		// Terran Wraith vs. Zerg Ultralisk
	matrix[8][40] = 25.04f;		// Terran Wraith vs. Zerg Broodling
	matrix[8][41] = 40.00f;		// Terran Wraith vs. Zerg Drone
	matrix[8][42] = 100.84f;		// Terran Wraith vs. Zerg Overlord
	matrix[8][43] = 1.14f;		// Terran Wraith vs. Zerg Mutalisk
	matrix[8][45] = 120.00f;		// Terran Wraith vs. Zerg Queen
	matrix[8][46] = 27.11f;		// Terran Wraith vs. Zerg Def;iler
	matrix[8][47] = 0.22f;		// Terran Wraith vs. Zerg Scourge
	matrix[8][50] = 60.00f;		// Terran Wraith vs. Zerg Inf;ested Terran
	matrix[8][58] = 1.05f;		// Terran Wraith vs. Terran Valkyrie
	matrix[8][60] = 0.76f;		// Terran Wraith vs. Protoss Corsair
	matrix[8][61] = 1.00f;		// Terran Wraith vs. Protoss Dark Templar
	matrix[8][62] = 3.21f;		// Terran Wraith vs. Zerg Devourer
	matrix[8][63] = 5.74f;		// Terran Wraith vs. Protoss Dark Archon
	matrix[8][64] = 13.39f;		// Terran Wraith vs. Protoss Probe
	matrix[8][65] = 66.94f;		// Terran Wraith vs. Protoss Zealot
	matrix[8][66] = 0.14f;		// Terran Wraith vs. Protoss Dragoon
	matrix[8][67] = 6.94f;		// Terran Wraith vs. Protoss High Templar
	matrix[8][68] = 0.01f;		// Terran Wraith vs. Protoss Archon
	matrix[8][69] = 0.67f;		// Terran Wraith vs. Protoss Shuttle
	matrix[8][70] = 0.01f;		// Terran Wraith vs. Protoss Scout
	matrix[8][72] = 200.83f;		// Terran Wraith vs. Protoss Carrier
	matrix[8][83] = 50.42f;		// Terran Wraith vs. Protoss Reaver
	matrix[8][84] = 1.00f;		// Terran Wraith vs. Protoss Observer
	matrix[8][103] = 119.83f;		// Terran Wraith vs. Zerg Lurker
	matrix[12][0] = 1.00f;		// Terran Battlecruiser vs. Terran Marine
	matrix[12][1] = 13.00f;		// Terran Battlecruiser vs. Terran Ghost
	matrix[12][2] = 80.00f;		// Terran Battlecruiser vs. Terran Vulture
	matrix[12][3] = 1.24f;		// Terran Battlecruiser vs. Terran Goliath
	matrix[12][5] = 150.00f;		// Terran Battlecruiser vs. Terran Siege Tank
	matrix[12][7] = 60.00f;		// Terran Battlecruiser vs. Terran SCV
	matrix[12][8] = 1.10f;		// Terran Battlecruiser vs. Terran Wraith
	matrix[12][9] = 200.00f;		// Terran Battlecruiser vs. Terran Science Vessel
	matrix[12][11] = 150.00f;		// Terran Battlecruiser vs. Terran Dropship
	matrix[12][12] = 0.98f;		// Terran Battlecruiser vs. Terran Battlecruiser
	matrix[12][32] = 50.00f;		// Terran Battlecruiser vs. Terran f;irebat
	matrix[12][34] = 60.00f;		// Terran Battlecruiser vs. Terran Medic
	matrix[12][37] = 1.00f;		// Terran Battlecruiser vs. Zerg Zergling
	matrix[12][38] = 1.77f;		// Terran Battlecruiser vs. Zerg Hydralisk
	matrix[12][39] = 400.00f;		// Terran Battlecruiser vs. Zerg Ultralisk
	matrix[12][40] = 30.00f;		// Terran Battlecruiser vs. Zerg Broodling
	matrix[12][41] = 40.00f;		// Terran Battlecruiser vs. Zerg Drone
	matrix[12][42] = 200.00f;		// Terran Battlecruiser vs. Zerg Overlord
	matrix[12][43] = 4.17f;		// Terran Battlecruiser vs. Zerg Mutalisk
	matrix[12][45] = 120.00f;		// Terran Battlecruiser vs. Zerg Queen
	matrix[12][46] = 80.00f;		// Terran Battlecruiser vs. Zerg Def;iler
	matrix[12][47] = 25.00f;		// Terran Battlecruiser vs. Zerg Scourge
	matrix[12][50] = 60.00f;		// Terran Battlecruiser vs. Zerg Inf;ested Terran
	matrix[12][58] = 2.04f;		// Terran Battlecruiser vs. Terran Valkyrie
	matrix[12][60] = 2.16f;		// Terran Battlecruiser vs. Protoss Corsair
	matrix[12][61] = 1.00f;		// Terran Battlecruiser vs. Protoss Dark Templar
	matrix[12][62] = 3.46f;		// Terran Battlecruiser vs. Zerg Devourer
	matrix[12][63] = 25.00f;		// Terran Battlecruiser vs. Protoss Dark Archon
	matrix[12][64] = 20.00f;		// Terran Battlecruiser vs. Protoss Probe
	matrix[12][65] = 100.00f;		// Terran Battlecruiser vs. Protoss Zealot
	matrix[12][66] = 1.98f;		// Terran Battlecruiser vs. Protoss Dragoon
	matrix[12][67] = 40.00f;		// Terran Battlecruiser vs. Protoss High Templar
	matrix[12][68] = 0.00f;		// Terran Battlecruiser vs. Protoss Archon
	matrix[12][69] = 80.00f;		// Terran Battlecruiser vs. Protoss Shuttle
	matrix[12][70] = 0.47f;		// Terran Battlecruiser vs. Protoss Scout
	matrix[12][72] = 300.00f;		// Terran Battlecruiser vs. Protoss Carrier
	matrix[12][83] = 100.00f;		// Terran Battlecruiser vs. Protoss Reaver
	matrix[12][84] = 1.00f;		// Terran Battlecruiser vs. Protoss Observer
	matrix[12][103] = 125.00f;	// Terran Battlecruiser vs. Zerg Lurker
	matrix[32][0] = 1.00f;		// Terran f;irebat vs. Terran Marine
	matrix[32][1] = 1.81f;		// Terran f;irebat vs. Terran Ghost
	matrix[32][2] = 0.54f;		// Terran f;irebat vs. Terran Vulture
	matrix[32][3] = 0.37f;		// Terran f;irebat vs. Terran Goliath
	matrix[32][5] = 0.42f;		// Terran f;irebat vs. Terran Siege Tank
	matrix[32][7] = 2.83f;		// Terran f;irebat vs. Terran SCV
	matrix[32][8] = 0.02f;		// Terran f;irebat vs. Terran Wraith
	matrix[32][9] = 1.00f;		// Terran f;irebat vs. Terran Science Vessel
	matrix[32][11] = 1.00f;		// Terran f;irebat vs. Terran Dropship
	matrix[32][12] = 0.02f;		// Terran f;irebat vs. Terran Battlecruiser
	matrix[32][32] = 0.92f;		// Terran f;irebat vs. Terran f;irebat
	matrix[32][34] = 60.00f;		// Terran f;irebat vs. Terran Medic
	matrix[32][37] = 1.00f;		// Terran f;irebat vs. Zerg Zergling
	matrix[32][38] = 0.42f;		// Terran f;irebat vs. Zerg Hydralisk
	matrix[32][39] = 0.02f;		// Terran f;irebat vs. Zerg Ultralisk
	matrix[32][40] = 5.00f;		// Terran f;irebat vs. Zerg Broodling
	matrix[32][41] = 3.33f;		// Terran f;irebat vs. Zerg Drone
	matrix[32][42] = 1.00f;		// Terran f;irebat vs. Zerg Overlord
	matrix[32][43] = 0.02f;		// Terran f;irebat vs. Zerg Mutalisk
	matrix[32][45] = 1.00f;		// Terran f;irebat vs. Zerg Queen
	matrix[32][46] = 80.00f;		// Terran f;irebat vs. Zerg Def;iler
	matrix[32][47] = 1.00f;		// Terran f;irebat vs. Zerg Scourge
	matrix[32][50] = 1.20f;		// Terran f;irebat vs. Zerg Inf;ested Terran
	matrix[32][58] = 1.00f;		// Terran f;irebat vs. Terran Valkyrie
	matrix[32][60] = 1.00f;		// Terran f;irebat vs. Protoss Corsair
	matrix[32][61] = 1.00f;		// Terran f;irebat vs. Protoss Dark Templar
	matrix[32][62] = 1.00f;		// Terran f;irebat vs. Zerg Devourer
	matrix[32][63] = 25.00f;		// Terran f;irebat vs. Protoss Dark Archon
	matrix[32][64] = 1.67f;		// Terran f;irebat vs. Protoss Probe
	matrix[32][65] = 0.02f;		// Terran f;irebat vs. Protoss Zealot
	matrix[32][66] = 1.06f;		// Terran f;irebat vs. Protoss Dragoon
	matrix[32][67] = 40.00f;		// Terran f;irebat vs. Protoss High Templar
	matrix[32][68] = 0.02f;		// Terran f;irebat vs. Protoss Archon
	matrix[32][69] = 1.00f;		// Terran f;irebat vs. Protoss Shuttle
	matrix[32][70] = 0.02f;		// Terran f;irebat vs. Protoss Scout
	matrix[32][72] = 1.00f;		// Terran f;irebat vs. Protoss Carrier
	matrix[32][83] = 100.00f;		// Terran f;irebat vs. Protoss Reaver
	matrix[32][84] = 1.00f;		// Terran f;irebat vs. Protoss Observer
	matrix[32][103] = 1.20f;		// Terran f;irebat vs. Zerg Lurker
	matrix[58][0] = 0.05f;		// Terran Valkyrie vs. Terran Marine
	matrix[58][1] = 0.16f;		// Terran Valkyrie vs. Terran Ghost
	matrix[58][2] = 1.00f;		// Terran Valkyrie vs. Terran Vulture
	matrix[58][3] = 0.00f;		// Terran Valkyrie vs. Terran Goliath
	matrix[58][5] = 1.00f;		// Terran Valkyrie vs. Terran Siege Tank
	matrix[58][7] = 1.00f;		// Terran Valkyrie vs. Terran SCV
	matrix[58][8] = 0.60f;		// Terran Valkyrie vs. Terran Wraith
	matrix[58][9] = 166.83f;		// Terran Valkyrie vs. Terran Science Vessel
	matrix[58][11] = 25.63f;		// Terran Valkyrie vs. Terran Dropship
	matrix[58][12] = 0.81f;		// Terran Valkyrie vs. Terran Battlecruiser
	matrix[58][32] = 1.00f;		// Terran Valkyrie vs. Terran f;irebat
	matrix[58][34] = 1.00f;		// Terran Valkyrie vs. Terran Medic
	matrix[58][37] = 1.00f;		// Terran Valkyrie vs. Zerg Zergling
	matrix[58][38] = 0.01f;		// Terran Valkyrie vs. Zerg Hydralisk
	matrix[58][39] = 1.00f;		// Terran Valkyrie vs. Zerg Ultralisk
	matrix[58][40] = 1.00f;		// Terran Valkyrie vs. Zerg Broodling
	matrix[58][41] = 1.00f;		// Terran Valkyrie vs. Zerg Drone
	matrix[58][42] = 67.33f;		// Terran Valkyrie vs. Zerg Overlord
	matrix[58][43] = 1.02f;		// Terran Valkyrie vs. Zerg Mutalisk
	matrix[58][45] = 120.00f;		// Terran Valkyrie vs. Zerg Queen
	matrix[58][46] = 1.00f;		// Terran Valkyrie vs. Zerg Def;iler
	matrix[58][47] = 0.20f;		// Terran Valkyrie vs. Zerg Scourge
	matrix[58][50] = 1.00f;		// Terran Valkyrie vs. Zerg Inf;ested Terran
	matrix[58][58] = 0.98f;		// Terran Valkyrie vs. Terran Valkyrie
	matrix[58][60] = 0.50f;		// Terran Valkyrie vs. Protoss Corsair
	matrix[58][61] = 1.00f;		// Terran Valkyrie vs. Protoss Dark Templar
	matrix[58][62] = 1.59f;		// Terran Valkyrie vs. Zerg Devourer
	matrix[58][63] = 1.00f;		// Terran Valkyrie vs. Protoss Dark Archon
	matrix[58][64] = 1.00f;		// Terran Valkyrie vs. Protoss Probe
	matrix[58][65] = 1.00f;		// Terran Valkyrie vs. Protoss Zealot
	matrix[58][66] = 0.02f;		// Terran Valkyrie vs. Protoss Dragoon
	matrix[58][67] = 1.00f;		// Terran Valkyrie vs. Protoss High Templar
	matrix[58][68] = 0.00f;		// Terran Valkyrie vs. Protoss Archon
	matrix[58][69] = 80.00f;		// Terran Valkyrie vs. Protoss Shuttle
	matrix[58][70] = 0.21f;		// Terran Valkyrie vs. Protoss Scout
	matrix[58][72] = 101.00f;		// Terran Valkyrie vs. Protoss Carrier
	matrix[58][83] = 1.00f;		// Terran Valkyrie vs. Protoss Reaver
	matrix[58][84] = 1.00f;		// Terran Valkyrie vs. Protoss Observer
	matrix[58][103] = 21.33f;		// Terran Valkyrie vs. Zerg Lurker
}

UCTGameState::~UCTGameState(void)
{
	for each(UCTGroup* g in this->myGroups) {delete g;}
	for each(UCTGroup* g in this->enemyGroups) {delete g;}
	groupActionMap.clear();
}
