#include "UCTManager.h"

//Used for testing specific heuristics. When enabled, will disable any UCT
//functionality and only make decisions based on heuristic value
//UCTManager::HeuristicType UCTManager::heuristicOnly = None;
//UCTManager::HeuristicType UCTManager::heuristicOnly = Closest;
//UCTManager::HeuristicType UCTManager::heuristicOnly = Random;
UCTManager::HeuristicType UCTManager::heuristicOnly = Isolated;

UCTManager::UCTManager(void)
{
	this->rootRoundNode = NULL;
	this->roundInProgress = false;
}

void UCTManager::evaluate()
{
	Utils::log("Entering UCTManager - evaluate", 1);

	int myUnitCount = WorldManager::Instance().myArmyVector->size();
	int enemyUnitCount = WorldManager::Instance().enemyHistoricalUnitMap.size();

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

	//Check for decision point (1 or more idle groups)
	unsigned int busyGroupCount = WorldManager::Instance().groupAttackMap.size();

	for each (std::vector<UnitGroup*>* groupVector in WorldManager::Instance().groupJoinVector)
	{
		busyGroupCount += groupVector->size();
	}

	if (this->roundInProgress && 
		myUnitCount > 0 &&
		busyGroupCount < WorldManager::Instance().myArmyGroups->size())
	{
		this->onDecisionPoint(NULL);
	}

	Utils::log("Leaving UCTManager - evaluate", 1);
}

void UCTManager::onRoundStart()
{
	Utils::log("Entering UCTManager - onRoundStart", 2);

	//Form groups
	this->formMyGroups();

	//Determine first starting node and initial team sizes
	this->maxFriendlyGroups = WorldManager::Instance().myArmyGroups->size();
	this->maxEnemyGroups = WorldManager::Instance().threatVector.size();
	this->rootRoundNode = new UCTNode(this->maxFriendlyGroups, this->maxEnemyGroups);

	//Manually trigger a decision point with the root node
	this->onDecisionPoint(this->rootRoundNode);

	Utils::log("Leaving UCTManager - onRoundStart", 2);
}

void UCTManager::onRoundEnd()
{
	Utils::log("Entering onRoundEnd", 2);

	//Clean up data structures
	WorldManager::Instance().groupAttackMap.clear();
	WorldManager::Instance().groupJoinVector.clear();
	this->rootRoundNode = NULL;

	Utils::log("Leaving onRoundEnd", 2);
}

void UCTManager::onDecisionPoint(UCTNode* rootNode)
{
	Utils::log("Entering UCTManager - onDecisionPoint", 2);

	//Handle cases where we want to only use heuristics
	if (UCTManager::heuristicOnly != None)
	{
		if (rootNode != NULL) {delete rootNode;}

		if (UCTManager::heuristicOnly == UCTManager::Closest)
		{
			this->heuristicClosest();	
		}
		else if (UCTManager::heuristicOnly == UCTManager::Random)
		{
			this->heuristicRandom();
		}
		else if (UCTManager::heuristicOnly == UCTManager::Isolated)
		{
			this->heuristicIsolated();
		}
		
		return;
	}

	Utils::log("UCTManager - onDecisionPoint - After heuristic check", 2);

	if (rootNode == NULL)
	{
		rootNode = new UCTNode(this->maxFriendlyGroups, this->maxEnemyGroups);
	}

	Utils::log("UCTManager - onDecisionPoint - After rootNode", 2);

	this->allNodes.insert(rootNode);

	for (int i=0; i<UCTManager::UCT_TOTAL_RUNS; i++)
	{
		//TODO: Could be more effecient and clone instead of generate each loop
		UCTGameState* gameState = this->getCurrentGameState();
		this->actionsTaken.clear();
		this->nodesVisited.clear();

		this->uctRun(rootNode, gameState, false);
	}

	Utils::log("UCTManager - onDecisionPoint - After UCT Runs", 2);

	//Do policy run to get set of initial actions with highest Q value
	this->actionsTaken.clear();
	this->nodesVisited.clear();
	UCTGameState* gameState = this->getCurrentGameState();
	this->uctRun(rootNode, gameState, true);
	delete gameState;

	Utils::log("UCTManager - onDecisionPoint - After Policy run", 2);

	this->executeActions(&this->actionsTaken);

	Utils::log("UCTManager - onDecisionPoint - After Action Execution", 2);

	if (Utils::debugLevel >= 5)
	{
		//Print out tree to log
		std::vector<std::string> stringVector;
		stringVector.push_back("UCTManager - Printout of UCT Decision Tree\n");
		this->getTreeStringBFS(rootNode, &stringVector);

		std::ostringstream stream;
		for each (std::string line in stringVector)
		{
			stream << line;
		}
		Utils::log(stream.str(), 5);
	}

	//Clean up allocated memory
	for each (UCTNode* node in this->allNodes)
	{
		delete node;
	}
	this->allNodes.clear();

	Utils::log("Leaving UCTManager - onDecisionPoint", 2);
}

void UCTManager::executeActions(std::vector<UCTAction*>* actionVector)
{
	//Note: We can match each action's group IDs with group indexs because of 
	//the deterministic way the initial game state was created for simulation
	for each (UCTAction* action in (*actionVector))
	{
		if (action->type == UCTAction::AttackAction)
		{
			UCTAttackAction* attackAction = (UCTAttackAction*)action;
			UnitGroup* myGroup = WorldManager::Instance().myArmyGroups->at(attackAction->myGroupIndex);
			Threat* threatToAttack = WorldManager::Instance().threatVector.at(attackAction->enemyGroupIndex);

			WorldManager::Instance().groupAttackMap[myGroup] = threatToAttack;
		}
		else if (action->type = UCTAction::JoinAction)
		{
			UCTJoinAction* joinAction = (UCTJoinAction*)action;
			std::vector<UnitGroup*>* groupVector = new std::vector<UnitGroup*>();

			for each (int index in joinAction->groupIdVector)
			{
				groupVector->push_back(
					WorldManager::Instance().myArmyGroups->at(index));
			}

			WorldManager::Instance().groupJoinVector.push_back(groupVector);
		}
	}
}

UCTGameState* UCTManager::getCurrentGameState()
{
	int count = 0;
	UCTGameState* newGameState = new UCTGameState();

	for each (UnitGroup* group in *(WorldManager::Instance().myArmyGroups))
	{
		newGameState->myGroups.push_back(new UCTGroup(count, group));
		count++;
	}

	count = 0;
	for each (Threat* threat in WorldManager::Instance().threatVector)
	{
		newGameState->enemyGroups.push_back(new UCTGroup(count, threat));
		count++;
	}

	return newGameState;
}

void UCTManager::uctRun(UCTNode* currentNode, UCTGameState* gameState, bool isPolicyRun)
{
	//If we have reached an end state (one of the forces is destoryed)
	if (gameState->isLeaf())
	{
		//Calculate final reward
		float reward = gameState->getRewardValue();

		//propagate back up tree
		for each (UCTAction* action in this->actionsTaken)
		{
			action->totalReward += reward;
			action->visitCount++;
		}

		for each (UCTNode* node in this->nodesVisited)
		{
			node->visitCount++;
		}
	}
	else
	{
		//Add node to visited list
		this->nodesVisited.push_back(currentNode);

		std::vector<UCTAction*> unexploredActions = std::vector<UCTAction*>();
		std::vector<UCTAction*> actionsToRemove = std::vector<UCTAction*>();
		std::vector<UCTAction*> validActions = std::vector<UCTAction*>();
		UCTAction* actionToExecute = NULL;
		bool hasUnexploredActions = false;

		for each (UCTAction* action in currentNode->possibleActions)
		{
			validActions.push_back(action);
		}

		//Trim actions down to those that can actually be performed
		for each (UCTAction* action in validActions)
		{
			if (! gameState->isValidAction(action))
			{
				actionsToRemove.push_back(action);
			}
		}

		for each (UCTAction* action in actionsToRemove)
		{
			Utils::vectorRemoveElement(&validActions, action);
		}

		//Determine unexplored actions.
		//Explore each action at least a set number of times
		for each(UCTAction* action in validActions)
		{
			if (action->visitCount == 0)
			{
				unexploredActions.push_back(action);
			}
			else if (action->visitCount < UCTManager::UCT_PER_ACTION_TRIES)
			{
				unexploredActions.push_back(action);
			}
		}

		//If there are unexplored actions (and we are not doing a policy run), randomly pick one
		//otherwise, pick one with highest Q value
		if ((! isPolicyRun) && unexploredActions.size() > 0)
		{
			int choice = rand() % unexploredActions.size();
			actionToExecute = unexploredActions.at(choice);
		}
		else
		{
			actionToExecute = getMaxQValue(currentNode, &validActions, isPolicyRun);
		}

		//Mark that we have take this action
		this->actionsTaken.push_back(actionToExecute);
		
		//Simulate action if needed
		//Otherwise, just tag busy groups and go to next state
		if (gameState->willTriggerSimulation(actionToExecute))
		{
			//Stop at first simulation point for policy run because we only
			//need the intial set of actions
			if (isPolicyRun)
			{
				return;
			}

			gameState->markGroupsForAction(actionToExecute);
			gameState->simulate();
		}
		else
		{
			gameState->markGroupsForAction(actionToExecute);
		}

		//Determine next node
		UCTNode* nextNode = actionToExecute->resultantNode;
		
		if (nextNode == NULL)
		{
			nextNode = new UCTNode(this->maxFriendlyGroups, this->maxEnemyGroups);
			this->allNodes.insert(nextNode);
			actionToExecute->resultantNode = nextNode;
		}

		this->uctRun(nextNode, gameState, isPolicyRun);
	}
}

void UCTManager::formMyGroups()
{
	Utils::log("Entering UCTManager - formMyGroups", 2);

	const int MAX_CLUSTER_DISTANCE = 100;
	std::map<BWAPI::Unit*, UnitGroup*> unitGroupMap;
	std::set<BWAPI::Unit*> myMen = std::set<BWAPI::Unit*>();
	std::vector<UnitGroup*>* unitGroupVector = new std::vector<UnitGroup*>();

	for each (BWAPI::Unit* unit in BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().canMove())
		{
			myMen.insert(unit);
		}
	}

	//TODO: Could be much more effecient
	//Determine groups using simple agglomerative hiearchial clustering
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

	Utils::log("Leaving UCTManager - formMyGroups", 2);
}

UCTAction* UCTManager::getMaxQValue(UCTNode* currentNode, std::vector<UCTAction*>* actionVector, bool isExploitOnly)
{
	UCTAction* bestAction = NULL;
	double highestQValue = 0;
	
	for each (UCTAction* action in (*actionVector))
	{
		//Exploitation
		double qValue = action->getAverageReward();

		//Exploration
		if (! isExploitOnly)
		{
			double explorationFactor = abs(qValue);
			double explorationBonus = explorationFactor * sqrt(log((double)currentNode->visitCount) / action->visitCount);
			qValue += explorationBonus;
		}

		if (bestAction == NULL || qValue > highestQValue)
		{
			bestAction = action;
			highestQValue = qValue;
		}
	}

	return bestAction;
}

void UCTManager::getTreeStringBFS(UCTNode* node, std::vector<std::string>* stringVector)
{
	if (node == NULL) {return;}

	std::stringstream stream;
	stream << "Node[" << node << "][n:" << node->visitCount << "]\n";
	stringVector->push_back(stream.str());

	for each (UCTAction* action in node->possibleActions)
	{
		if (action->visitCount > 0)
		{
			stream.str("");
			stream << "  Action" << action->toString() << "\n";
			stringVector->push_back(stream.str());
		}
	}

	for each (UCTAction* action in node->possibleActions)
	{
		if (action->resultantNode != NULL)
		{
			this->getTreeStringBFS(action->resultantNode, stringVector);
		}
	}
}

void UCTManager::heuristicClosest()
{
	for each (UnitGroup* myGroup in (*WorldManager::Instance().myArmyGroups))
	{
		if (WorldManager::Instance().groupAttackMap[myGroup] == NULL)
		{
			BWAPI::Position fromPos = myGroup->getCentroid();
			double minDist = 0;
			Threat* closestThreat = NULL;

			for each (Threat* threat in WorldManager::Instance().threatVector)
			{
				double dist = fromPos.getDistance(threat->getCentroid());

				if (closestThreat == NULL || dist < minDist)
				{
					closestThreat = threat;
					minDist = dist;
				}
			}

			WorldManager::Instance().groupAttackMap[myGroup] = closestThreat;
		}
	}
}


void UCTManager::heuristicRandom()
{
	std::vector<UCTAction*> actionsTaken;
	UCTNode* currentNode = new UCTNode(this->maxFriendlyGroups, this->maxEnemyGroups);
	UCTGameState* gameState = this->getCurrentGameState();
	int freeGroupCount = gameState->myGroups.size() - gameState->groupActionMap.size();

	while (freeGroupCount > 0)
	{
		std::vector<UCTAction*> actionsToRemove = std::vector<UCTAction*>();
		std::vector<UCTAction*> validActions = std::vector<UCTAction*>();
		UCTAction* actionToExecute = NULL;

		for each (UCTAction* action in currentNode->possibleActions)
		{
			validActions.push_back(action);
		}

		//Trim actions down to those that can actually be performed
		for each (UCTAction* action in validActions)
		{
			if (! gameState->isValidAction(action))
			{
				actionsToRemove.push_back(action);
			}
		}

		for each (UCTAction* action in actionsToRemove)
		{
			Utils::vectorRemoveElement(&validActions, action);
		}

		int choice = rand() % validActions.size();
		actionToExecute = validActions.at(choice);
		actionsTaken.push_back(actionToExecute);
		gameState->markGroupsForAction(actionToExecute);

		freeGroupCount = gameState->myGroups.size() - gameState->groupActionMap.size();
	}

	this->executeActions(&actionsTaken);
	delete gameState;
	delete currentNode;
}

void UCTManager::heuristicIsolated()
{
	Threat* isolatedThreat = NULL;
	double maxDist = 0;

	//Get threat that is farthest away from all othe threats (most isolated)
	for each (Threat* threat in WorldManager::Instance().threatVector)
	{
		BWAPI::Position basePos = threat->getCentroid();
		double dist = 0;

		for each (Threat* testThreat in WorldManager::Instance().threatVector)
		{
			if (testThreat != threat)
			{
				dist += basePos.getDistance(testThreat->getCentroid());
			}
		}

		if (isolatedThreat == NULL || dist > maxDist)
		{
			isolatedThreat = threat;
			maxDist = dist;
		}
	}

	for each (UnitGroup* myGroup in (*WorldManager::Instance().myArmyGroups))
	{
		if (WorldManager::Instance().groupAttackMap[myGroup] == NULL)
		{
			WorldManager::Instance().groupAttackMap[myGroup] = isolatedThreat;
		}
	}
}

UCTManager::~UCTManager(void)
{
}