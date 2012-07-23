#include "UnitManager.h"
#include "AdjutantAIModule.h"

UnitManager::UnitManager(void)
{
	this->buildQueue = new BuildQueue();
	this->buildOrder = LoadBuildOrder::Instance().getBuildOrder();
	this->buildOrderBuilding = BWAPI::UnitTypes::None;
}

void UnitManager::evalute()
{
	if (! WorldManager::Instance().isTerrainAnalyzed)
	{
		this->manageStartOfGame();
	}
	else
	{
		//Construct supply depots and make sure we don't get supply blocked
		this->manageSupply();

		//Assign workers to minerals and train more if needed. Also, build refineries.
		this->manageResourceGathering();

		this->manageBuildOrder();
	}

	//For tasks that are ready, transfer them to the global vector so that
	//the build manager will build them
	int remainingMinerals = BWAPI::Broodwar->self()->minerals() - WorldManager::Instance().reservedMinerals;
	int remainingGas = BWAPI::Broodwar->self()->gas() - WorldManager::Instance().reservedMinerals;
	int remainingSupply = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
	
	std::vector<BuildTask*> tasksToAddBack = std::vector<BuildTask*>();

	while(! buildQueue->getPriorityQueue()->empty())
	{
		BuildTask* task = buildQueue->removeTop();

		if (task->isReady(remainingMinerals, remainingGas, remainingSupply))
		{
			WorldManager::Instance().buildTaskVector.push_back(task);
			task->updateResourceCost(&remainingMinerals, &remainingGas, &remainingSupply);
		}
		else
		{
			tasksToAddBack.push_back(task);
			task->updateResourceCost(&remainingMinerals, &remainingGas, &remainingSupply);
		}
	}

	for each (BuildTask* task in tasksToAddBack)
	{
		buildQueue->push(task);
	}

	std::vector<std::string> textVector = std::vector<std::string>();
	std::stringstream stream;
	stream << "Build Queue(" << tasksToAddBack.size() << ")";
	stream << " at frame " << BWAPI::Broodwar->getFrameCount();
	stream << " Now=" << BWAPI::Broodwar->getFrameCount();
	std::string titleText = stream.str();

	textVector.push_back(titleText);
	for each (BuildTask* task in tasksToAddBack)
	{
		textVector.push_back(task->toString());
	}

	textVector.push_back("In BuildTaskVector");
	for each (BuildTask* task in WorldManager::Instance().buildTaskVector)
	{
		textVector.push_back(task->toString());
	}

	for(int i=0; i<(int)textVector.size(); i++)
	{
		BWAPI::Broodwar->drawTextScreen(200, 16*i, textVector.at(i).c_str());
	}
}

void UnitManager::manageStartOfGame()
{
	//Very very beginning of the game
	BWAPI::Unit* cc = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Command_Center].front();

	if (cc != NULL && ! cc->isTraining() && BWAPI::Broodwar->canMake(NULL, BWAPI::UnitTypes::Terran_SCV))
	{
		buildQueue->push(new BuildTask(500, BWAPI::UnitTypes::Terran_SCV, cc));
	}

	//Assign initial workers to minerals
	std::vector<BWAPI::Unit*> availableMinerals = std::vector<BWAPI::Unit*>();

	for each (BWAPI::Unit* mineral in BWAPI::Broodwar->getMinerals())
	{
		availableMinerals.push_back(mineral);
	}
	
	for each (BWAPI::Unit* worker in (*WorldManager::Instance().myWorkerVector))
	{
		if (worker->isIdle())
		{
			BWAPI::Unit* targetMineral = NULL;

			for each (BWAPI::Unit* mineral in availableMinerals)
			{
				if (targetMineral == NULL || 
					worker->getDistance(mineral) < worker->getDistance(targetMineral))
				{
					targetMineral = mineral;
				}
			}

			if (targetMineral != NULL)
			{
				worker->gather(targetMineral);
				Utils::vectorRemoveElement(&availableMinerals, targetMineral);
			}
		}
	}
}

void UnitManager::manageResourceGathering()
{
	BWAPI::Unit* cc = NULL;
	BWAPI::Unit* refinery = NULL;
	int gasWorkers = 0;
	int mineralWorkers = 0;

	if (! WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Refinery].empty())
	{
		refinery = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Refinery].front();
	}
	
	if (! WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Command_Center].empty())
	{
		cc = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Command_Center].front();
	}

	for each(BWAPI::Unit* worker in (*WorldManager::Instance().myWorkerVector))
	{
		if (worker->isGatheringMinerals()) { mineralWorkers++; }
		if (worker->isGatheringGas()) { gasWorkers++; }
	}

	//Assign idle workers
	for each(BWAPI::Unit* unit in WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_SCV])
	{
		if (unit->isIdle())
		{
			// 6 Mineral workers, 3 Gas Workers
			if (mineralWorkers >= 6 && gasWorkers < 3 
				&& refinery != NULL && refinery->isCompleted())
			{	
				// Send to refinery for gas gathering
				unit->gather(refinery);
				gasWorkers++;
			}
			else
			{
				BWAPI::Unit* closestMineral = NULL;
				if (WorldManager::Instance().myHomeBase->minerals() > 0)
				{
					closestMineral = Utils::getClosestUnit(unit, &WorldManager::Instance().myHomeBase->getMinerals());
				}
				else
				{
					closestMineral = Utils::getClosestUnit(unit, &BWAPI::Broodwar->getMinerals());
				}

				unit->rightClick(closestMineral);
				mineralWorkers++;
			}
		}
	}
	
	//Transfer existing workers to gas if needed
	for each(BWAPI::Unit* worker in (*WorldManager::Instance().myWorkerVector))
	{
		if (mineralWorkers >= 10 && gasWorkers < 3 
			&& refinery != NULL && refinery->isCompleted())
		{	
			// Transfer worker to gas to refinery for gas gathering
			worker->gather(refinery);
			mineralWorkers--;
			gasWorkers++;
			break;
		}
	}

	// Create refinery Action
	if (BWAPI::Broodwar->getFrameCount() > 3000
		&& WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Refinery].empty()
		&& this->inPipelineCount(BWAPI::UnitTypes::Terran_Refinery) == 0)
	{
		BWAPI::Unit* geyser = (*WorldManager::Instance().myHomeBase->getGeysers().begin());
		this->buildQueue->push(new BuildTask(300, BWAPI::Broodwar->self()->getRace().getRefinery(), geyser->getTilePosition()));
	}

	//train workers
	unsigned int numMineralPatches = WorldManager::Instance().myHomeBase->getMinerals().size();
	unsigned int numGasGeysers = WorldManager::Instance().myHomeBase->getGeysers().size();
	
	if (cc != NULL && ! cc->isTraining()
		&& this->buildQueue->getScheduledCount(BWAPI::UnitTypes::Terran_SCV) == 0
		&& WorldManager::Instance().myWorkerVector->size() < (numMineralPatches * 2) + (numGasGeysers * 3)
		+ 3 //3 extra workers for construction
		)
	{
		this->buildQueue->push(new BuildTask(350, BWAPI::UnitTypes::Terran_SCV, cc));
	}
}

//Construct supply depots and make sure we don't get supply blocked
//Supply is doubled from expected value for lings/scourges (i.e. cap = 400)
void UnitManager::manageSupply()
{
	if (BWAPI::Broodwar->self()->supplyTotal() >= 200*2)
	{
		return; //nothing to do here
	}

	//supply depots
	int supplyRemaining = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
	int totalSupplyInProcess = this->inPipelineCount(BWAPI::UnitTypes::Terran_Supply_Depot);

	if (supplyRemaining + (totalSupplyInProcess * BWAPI::UnitTypes::Terran_Supply_Depot.supplyProvided()) <= 0)
	{
		//Handle supply block
		buildQueue->push(new BuildTask(-1, BWAPI::UnitTypes::Terran_Supply_Depot));
	}
	else if ((BWAPI::Broodwar->getFrameCount() > 1000 && supplyRemaining < 5*2)
		|| (BWAPI::Broodwar->getFrameCount() > 8000 && supplyRemaining < 15*2))
	{
		//TODO: Enhance - use production estimation instead of set rules
		//Plan ahead
		if (totalSupplyInProcess == 0)
		{
			buildQueue->push(new BuildTask(300, BWAPI::UnitTypes::Terran_Supply_Depot));
		}
	}
}

void UnitManager::manageBuildOrder()
{
	//See if we should be using next part of build order
	this->buildOrder->checkForTransition();

	//Check to see if scheduled units are complete
	bool buildingFree = true;

	if (this->buildOrderBuilding != BWAPI::UnitTypes::None)
	{
		if (inPipelineCount(this->buildOrderBuilding) > 0)
		{
			buildingFree = false;
		}
		else
		{
			//Building must have completed (or interrupted)
			this->buildOrderBuilding = BWAPI::UnitTypes::None;
		}
	}

	BuildOrderUnits* orderUnits = this->buildOrder->getCurrentUnits();
	std::map<BWAPI::UnitType, float> unitRatioMap = orderUnits->getUnitRatioNormalized();
	std::map<BWAPI::UnitType, float> whatBuildsRatioMap = orderUnits->getWhatBuildsNormalized();
	std::map<BWAPI::UnitType, int> expectedPersonMap = std::map<BWAPI::UnitType, int>();
	int totalUnitCount = 0;

	//Get all of our current units
	for each (std::pair<BWAPI::UnitType, float> pair in unitRatioMap)
	{
		totalUnitCount += BWAPI::Broodwar->self()->completedUnitCount(pair.first)
			+ this->inPipelineCount(pair.first);;
	}
	
	//Figure out what we should build next
	for each (std::pair<BWAPI::UnitType, float> pair in unitRatioMap)
	{
		BWAPI::UnitType unitType = pair.first;

		int expectedUnitCount = (int)std::ceil(totalUnitCount * pair.second); //round up
		int currentUnitCount = BWAPI::Broodwar->self()->completedUnitCount(unitType)
			+ this->inPipelineCount(pair.first);
		int toBuildTest = expectedUnitCount - currentUnitCount;

		//If we can make the unit, train it; otherwise, build a required building for it
		if (Utils::canMakeGivenUnits(unitType))
		{
			expectedPersonMap[pair.first] = toBuildTest;
		}
		else
		{
			if (buildingFree)
			{
				BWAPI::UnitType potentialBuildingType = this->getNextRequiredUnit(unitType);

				if (potentialBuildingType != BWAPI::UnitTypes::None)
				{
					this->buildQueue->push(new BuildTask(500, potentialBuildingType));
					this->buildOrderBuilding = potentialBuildingType;
					buildingFree = false;
				}
			}
		}
	}

	if (! expectedPersonMap.empty())
	{
		int totalExpectedPerson = 0;

		for each (std::pair<BWAPI::UnitType, int> pair in expectedPersonMap)
		{
			totalExpectedPerson += abs(pair.second);
		}

		for each (std::pair<BWAPI::UnitType, int> pair in expectedPersonMap)
		{
			BWAPI::UnitType unitType = pair.first;
			float expectedRatio = 0;
			
			if (totalExpectedPerson != 0)
			{
				expectedRatio = (float)pair.second/(float)totalExpectedPerson;
			}

			if (this->inPipelineCount(unitType, false) == 0)
			{
				//Compute priority based on how much we expected to see this unit
				int priority = 500 + (int)(-60.0 * expectedRatio);
				this->buildQueue->push(new BuildTask(priority, unitType));
			}
		}
	}

	//If we have all of the required buildings, build more production buildings
	if (buildingFree)
	{
		BWAPI::UnitType buildingToBuild = BWAPI::UnitTypes::None;
		totalUnitCount = 0;
		int toBuildCount = INT_MIN;

		for each (std::pair<BWAPI::UnitType, float> pair in whatBuildsRatioMap)
		{
			totalUnitCount += BWAPI::Broodwar->self()->completedUnitCount(pair.first)
				+ this->inPipelineCount(pair.first);
		}

		for each (std::pair<BWAPI::UnitType, float> pair in whatBuildsRatioMap)
		{
			BWAPI::UnitType unitType = pair.first;

			int expectedUnitCount = (int)std::ceil(totalUnitCount * pair.second); //round up
			int currentUnitCount = BWAPI::Broodwar->self()->completedUnitCount(unitType)
				+ this->inPipelineCount(unitType);
			int toBuildTest = expectedUnitCount - currentUnitCount;

			if (Utils::canMakeGivenUnits(unitType)
				&& toBuildTest > toBuildCount)
			{
				toBuildCount = toBuildTest;
				buildingToBuild = pair.first;
			}
		}

		if (buildingToBuild != BWAPI::UnitTypes::None)
		{
			this->buildQueue->push(new BuildTask(510, buildingToBuild));
			this->buildOrderBuilding = buildingToBuild;
		}
	}
}

BWAPI::UnitType UnitManager::getNextRequiredUnit(BWAPI::UnitType unitType)
{
	if (unitType.requiredUnits().size() == 0)
	{
		return BWAPI::UnitTypes::None;
	}

	//First, add this level of required units
	for each (std::pair<BWAPI::UnitType, int> typePair in unitType.requiredUnits())
	{
		if (this->inPipelineCount(typePair.first) == 0
			&& BWAPI::Broodwar->self()->completedUnitCount(typePair.first) == 0
			&& Utils::canMakeGivenUnits(typePair.first))
		{
			return typePair.first;
		}
	}

	//Then add the next level
	for each (std::pair<BWAPI::UnitType, int> typePair in unitType.requiredUnits())
	{
		BWAPI::UnitType ut = getNextRequiredUnit(typePair.first);
		if (ut != BWAPI::UnitTypes::None)
		{
			return ut;
		}
	}

	return BWAPI::UnitTypes::None;
}

int UnitManager::inPipelineCount(BWAPI::UnitType unitType)
{
	return this->inPipelineCount(unitType, true);
}

int UnitManager::inPipelineCount(BWAPI::UnitType unitType, bool includeIncomplete)
{
	int count = 0;

	//In queue to be built
	count += this->buildQueue->getScheduledCount(unitType);
	
	//In between plan and scheduling
	for each (BuildTask* buildTask in WorldManager::Instance().buildTaskVector)
	{
		if (buildTask->unitType == unitType)
		{
			count++;
		}
	}

	//Worker on way to build it
	count += WorldManager::Instance().imminentBuildingMap[unitType];

	if (includeIncomplete)
	{
		count += BWAPI::Broodwar->self()->incompleteUnitCount(unitType);
	}

	return count;
}

UnitManager::~UnitManager(void)
{
	delete this->buildQueue;
}
