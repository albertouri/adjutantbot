#include "UnitManager.h"
#include "AdjutantAIModule.h"

UnitManager::UnitManager(void)
{
	this->buildQueue = new BuildQueue();
}

void UnitManager::evalute()
{
	//Assign workers to minerals and train more if needed. Also, build refineries.
	this->manageResourceGathering();
	this->manageSupply();

	if (! WorldManager::Instance().isTerrainAnalyzed)
	{
		//Very very beginning of the game
		BWAPI::Unit* cc = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Command_Center].front();

		if (cc != NULL && ! cc->isTraining())
		{
			buildQueue->push(new BuildTask(50, BWAPI::UnitTypes::Terran_SCV, cc));
		}
	}
	else
	{
		if (BWAPI::Broodwar->getFrameCount() % 500 == 0) {
			BWAPI::Broodwar->printf("Reserved %d/%d", WorldManager::Instance().reservedMinerals, WorldManager::Instance().reservedGas);
		}

		if (! WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Barracks].empty())
		{
			for each (BWAPI::Unit* rax in WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Barracks])
			{
				if (rax->isCompleted() && ! rax->isTraining())
				{
					bool useModeling = AdjutantAIModule::useOpponentModeling;
					int choice = 0;

					std::vector<BWAPI::Unit*> academyVector = 
						WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Academy];

					//If we have an academy, randomly choose; otherwise, default to marine
					if (! academyVector.empty() && ! academyVector.front()->isBeingConstructed())
					{
						choice = rand() % 90; //0 - 89
					}

					if (useModeling)
					{
						double rangeWeight = WorldManager::Instance().getEnemyRangedWeight();
						int rangeOffset = (int)(40.0 * rangeWeight);

						if (choice >= 80)
						{
							buildQueue->push(
								new BuildTask(50, BWAPI::UnitTypes::Terran_Medic, rax));
						}
						else if (choice >= 30 + rangeOffset)
						{
							buildQueue->push(
								new BuildTask(50, BWAPI::UnitTypes::Terran_Firebat, rax));
						}
						else
						{
							buildQueue->push(
								new BuildTask(50, BWAPI::UnitTypes::Terran_Marine, rax));
						}
					}
					else
					{
						if (choice >= 60)
						{
							buildQueue->push(
								new BuildTask(50, BWAPI::UnitTypes::Terran_Medic, rax));
						}
						else if (choice >= 30)
						{
							buildQueue->push(
								new BuildTask(50, BWAPI::UnitTypes::Terran_Firebat, rax));
						}
						else
						{
							buildQueue->push(
								new BuildTask(50, BWAPI::UnitTypes::Terran_Marine, rax));
						}
					}
				}
			}
		}

		//barracks
		int raxBuilding = buildQueue->getScheduledCount(BWAPI::UnitTypes::Terran_Barracks);

		if (raxBuilding == 0
			|| (BWAPI::Broodwar->self()->minerals() > 300 && raxBuilding < 2))
		{
			buildQueue->push(new BuildTask(60, BWAPI::UnitTypes::Terran_Barracks));
		}

		//academy
		if (BWAPI::Broodwar->getFrameCount() > 3000 && WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Academy].empty())
		{
			int acadBuilding = buildQueue->getScheduledCount(BWAPI::UnitTypes::Terran_Academy);

			if (acadBuilding == 0 && BWAPI::Broodwar->canMake(NULL, BWAPI::UnitTypes::Terran_Academy))
			{
				buildQueue->push(new BuildTask(50, BWAPI::UnitTypes::Terran_Academy));
			}
		}
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

void UnitManager::manageResourceGathering()
{
	//manually assigning idle workers to mine
	//TODO:Update to use worldManager and actions instead
	BWAPI::Unit* cc = NULL;
	BWAPI::Unit* refinery = NULL;
	BWAPI::Unit* closestGeyser=NULL;
	BWAPI::Unit* closestMineral=NULL;

	int gasWorkers = 0;
	int mineralWorkers = 0;

	// Build world dynamics for meterics and understanding
	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		if ((*unit)->getType().isWorker()) 
		{
			if ((*unit)->isGatheringMinerals() || (*unit)->isCarryingMinerals()) { mineralWorkers++; }
			if ((*unit)->isGatheringGas() || (*unit)->isCarryingGas()) { gasWorkers++; }
		}

		// Refinery
		else if ((*unit)->getType().isRefinery()) 
		{
			refinery = (*unit);
		}

		// Command Center
		else if ((*unit)->getType().isResourceDepot())
		{
			cc = (*unit);

			// Find closest Geysers
			for(std::set<BWAPI::Unit*>::iterator g=BWAPI::Broodwar->getGeysers().begin();g!=BWAPI::Broodwar->getGeysers().end();g++)
			{
				if (closestGeyser==NULL || (*unit)->getDistance(*g) < (*unit)->getDistance(closestGeyser))
				{
					closestGeyser=*g;
				}
			}
			
			for(std::set<BWAPI::Unit*>::iterator m=BWAPI::Broodwar->getMinerals().begin();m!=BWAPI::Broodwar->getMinerals().end();m++)
			{
				if (closestMineral==NULL || (*unit)->getDistance(*m) < (*unit)->getDistance(closestMineral))
				{
					closestMineral=*m;
				}
			}
		}
	}

	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		if ((*unit)->getType().isWorker())
		{
			if ( (*unit)->isIdle() )
			{
				// 6 Mineral workers, 3 Gas Workers
				if (mineralWorkers >= 6 && gasWorkers <= 3 
						&& closestGeyser!=NULL && closestGeyser->getType().isRefinery() && closestGeyser->isCompleted())
				{	
					// Send to refinery for gas gathering
					(*unit)->rightClick(closestGeyser);
					gasWorkers++;
				}
				else if (closestMineral!=NULL)
				{
					(*unit)->rightClick(closestMineral);
					mineralWorkers++;
				}
			}
		}
	}
	
	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		// If is Command Center
		if ((*unit)->getType().isResourceDepot())
		{
			// Find closest Geyser
			BWAPI::Unit* closestGeyser=NULL;
			for(std::set<BWAPI::Unit*>::iterator g=BWAPI::Broodwar->getGeysers().begin();g!=BWAPI::Broodwar->getGeysers().end();g++)
			{
				if (closestGeyser==NULL || (*unit)->getDistance(*g) < (*unit)->getDistance(closestGeyser))
				{
					closestGeyser=*g;
				}
			}

			if ( closestGeyser!=NULL && (!closestGeyser->getType().isRefinery() || !closestGeyser->isBeingConstructed()) )
			{
				// Create refinerey Action
				if (BWAPI::Broodwar->getFrameCount() > 3000
					&& WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Refinery].empty()
					&& this->buildQueue->getScheduledCount(BWAPI::UnitTypes::Terran_Refinery) == 0)
				{
					this->buildQueue->push(new BuildTask(40, BWAPI::Broodwar->self()->getRace().getRefinery(), closestGeyser->getTilePosition()));
				}
			}
		}
	}


	//train workers
	if (WorldManager::Instance().isTerrainAnalyzed)
	{
		unsigned int numMineralPatches = WorldManager::Instance().myHomeBase->getMinerals().size();
		unsigned int numGasGeysers = WorldManager::Instance().myHomeBase->getGeysers().size();

		
		if (cc != NULL && (! cc->isTraining())
			&& this->buildQueue->getScheduledCount(BWAPI::UnitTypes::Terran_SCV) == 0
			&& WorldManager::Instance().myWorkerVector->size() < (numMineralPatches * 2) + (numGasGeysers * 3)
			+ 3 //3 extra workers for construction
			)
		{
			this->buildQueue->push(new BuildTask(50, BWAPI::UnitTypes::Terran_SCV, cc));
		}
	}
}

void UnitManager::manageSupply()
{
	if (BWAPI::Broodwar->self()->supplyTotal() >= 200)
	{
		return; //nothing to do here
	}

	//supply depots
	int supplyRemaining = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
	int supplyInQueue = buildQueue->getScheduledCount(BWAPI::UnitTypes::Terran_Supply_Depot);
	int supplyBeingConstructed = 0;
	int supplyAboutToBeConstructed = 0;

	//Captures supply deploys currently being constructed
	for each (BWAPI::Unit* supply in WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Supply_Depot])
	{
		if (supply->isBeingConstructed())
		{
			supplyBeingConstructed++;
		}
	}
	
	//Captures workers that have been ordered to construct something but haven't started yet
	for each (std::pair<BWAPI::Unit*, ConstructBuildingAction*> pair in WorldManager::Instance().workersBuildingMap)
	{
		ConstructBuildingAction* task = pair.second;
		
		if (task->buildingType.getID() == BWAPI::UnitTypes::Terran_Supply_Depot.getID())
		{
			supplyAboutToBeConstructed++;
		}
	}

	if ( (BWAPI::Broodwar->getFrameCount() > 500 && supplyRemaining < 5)
		|| (BWAPI::Broodwar->getFrameCount() > 8000 && supplyRemaining < 15)
		)
	{
		if (supplyInQueue == 0 && supplyBeingConstructed == 0 && supplyAboutToBeConstructed == 0)
		{
			buildQueue->push(new BuildTask(40, BWAPI::UnitTypes::Terran_Supply_Depot));
		}
	}
}

UnitManager::~UnitManager(void)
{
	delete this->buildQueue;
}
