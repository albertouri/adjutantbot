#include "MacroModule.h"
#include "AdjutantAIModule.h"

MacroModule::MacroModule(void)
{
}

void MacroModule::evalute(WorldModel* worldModel, ActionQueue* actionQueue)
{
	
	BuildingManager::evalute(worldModel, actionQueue);
	//examine world model
	//add actions to actionQueue based on current state

	//manually assigning idle workers to mine
	//TODO:Update to use worldModel and actions instead
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
			/*
			// Worker is gathering something
			else if ( !(*unit)->isIdle() && !(*unit)->isConstructing()) // Else back to the mines with the worker!
			{		
				if (mineralWorkers > 5 && gasWorkers < 3 && refinery!=NULL && !refinery->isBeingConstructed())
				{	
					(*unit)->rightClick(refinery);
					gasWorkers++;
				}
			}
			*/
		}
	}
	

	//start temporary unit training/construction
	if (worldModel->isTerrainAnalyzed)
	{
		if (BWAPI::Broodwar->getFrameCount() % 500 == 0) {
			BWAPI::Broodwar->printf("Reserved %d/%d", worldModel->reservedMinerals, worldModel->reservedGas);
		}

		//train workers
		unsigned int numMineralPatches = worldModel->myHomeBase->getMinerals().size();
		unsigned int numGasGeysers = worldModel->myHomeBase->getGeysers().size();

		if (cc != NULL && ! cc->isTraining() && 
			worldModel->myWorkerVector->size() < (numMineralPatches * 2) + (numGasGeysers * 3)
			+ 3 //3 extra workers for construction
			)
		{	
			actionQueue->push(new TrainUnitAction(50, cc, BWAPI::Broodwar->self()->getRace().getWorker()));
		}

		if (worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Barracks] != NULL)
		{
			for each (BWAPI::Unit* rax in (*worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Barracks]))
			{
				if (! rax->isTraining())
				{
					bool useModeling = AdjutantAIModule::useOpponentModeling;
					int choice = 0;

					std::vector<BWAPI::Unit*>* academyVector = 
						worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Academy];

					//If we have an academy, randomly choose; otherwise, default to marine
					if (academyVector != NULL && ! academyVector->front()->isBeingConstructed())
					{
						choice = rand() % 90; //0 - 89
					}

					if (useModeling)
					{
						double rangeWeight = worldModel->getEnemyRangedWeight();
						int rangeOffset = (int)(40.0 * rangeWeight);

						if (choice >= 80)
						{
							actionQueue->push(
								new TrainUnitAction(50, rax, BWAPI::UnitTypes::Terran_Medic));
						}
						else if (choice >= 30 + rangeOffset)
						{
							actionQueue->push(
								new TrainUnitAction(50, rax, BWAPI::UnitTypes::Terran_Firebat));
						}
						else
						{
							actionQueue->push(
								new TrainUnitAction(50, rax, BWAPI::UnitTypes::Terran_Marine));
						}
					}
					else
					{
						if (choice >= 60)
						{
							actionQueue->push(
								new TrainUnitAction(50, rax, BWAPI::UnitTypes::Terran_Medic));
						}
						else if (choice >= 30)
						{
							actionQueue->push(
								new TrainUnitAction(50, rax, BWAPI::UnitTypes::Terran_Firebat));
						}
						else
						{
							actionQueue->push(
								new TrainUnitAction(50, rax, BWAPI::UnitTypes::Terran_Marine));
						}
					}
				}
			}
		}

		std::vector<BWAPI::UnitType> toBuildVector = std::vector<BWAPI::UnitType>();
		
		//supply depots
		int supplyRemaining = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
		if ( (BWAPI::Broodwar->getFrameCount() > 500 && supplyRemaining < 5)
			|| (BWAPI::Broodwar->getFrameCount() > 8000 && supplyRemaining < 15)
			)
		{
			int supplyInQueue = actionQueue->countBuildingActions(BWAPI::UnitTypes::Terran_Supply_Depot);			
			int supplyBeingConstructed = 0;

			if (worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Supply_Depot] != NULL)
			{
				for each (BWAPI::Unit* supply in (*worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Supply_Depot]))
				{
					if (supply->isBeingConstructed())
					{
						supplyBeingConstructed++;
					}
				}
			}

			if (supplyInQueue == 0 && supplyBeingConstructed == 0)
			{
				toBuildVector.push_back(BWAPI::UnitTypes::Terran_Supply_Depot);
			}
		}

		//barracks
		int raxBuilding = actionQueue->countBuildingActions(BWAPI::UnitTypes::Terran_Barracks);

		if (raxBuilding == 0
			|| (BWAPI::Broodwar->self()->minerals() > 300 && raxBuilding < 2))
		{
			toBuildVector.push_back(BWAPI::UnitTypes::Terran_Barracks);
		}

		//academy
		if (BWAPI::Broodwar->getFrameCount() > 3000 && worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Academy] == NULL)
		{
			int acadBuilding = actionQueue->countBuildingActions(BWAPI::UnitTypes::Terran_Academy);

			if (acadBuilding == 0 && BWAPI::Broodwar->canMake(NULL, BWAPI::UnitTypes::Terran_Academy))
			{
				toBuildVector.push_back(BWAPI::UnitTypes::Terran_Academy);
			}
		}

		//comsat
		if (BWAPI::Broodwar->getFrameCount() > 3000)
		{
			std::vector<BWAPI::Unit*>* comandCenters = worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Command_Center];

			if (comandCenters != NULL)
			{
				for each (BWAPI::Unit* cc in (*comandCenters))
				{
					if (cc->getAddon() == NULL)
					{
						cc->buildAddon(BWAPI::UnitTypes::Terran_Comsat_Station);
					}
				}
			}
		}

		//Construct buildings randomly
		for each (BWAPI::UnitType buildingType in toBuildVector)
		{
			bool isValidPosition = true;
			int counter = 0;

			BWAPI::Unit* freeWorker = Utils::getFreeWorker(worldModel->myWorkerVector);
			
			if (freeWorker != NULL)
			{
				BWAPI::TilePosition buildingTile = worldModel->myHomeBase->getTilePosition();

				do
				{
					//Start at command center + some random offset from the CC
					//gradually expand build radious
					buildingTile = worldModel->myHomeBase->getTilePosition();
					int range = 20 + (int)(30.0*(counter/100));
					buildingTile.x() += (rand() % (range*2)) - range; 
					buildingTile.y() += (rand() % (range*2)) - range;
					
					isValidPosition = Utils::isValidBuildingLocation(buildingTile, buildingType);

					if (counter > 100) {break;}
					counter++;	
				} while (! isValidPosition);

				if (counter > 100)
				{
					BWAPI::Broodwar->sendText("Unable to find location for building");
				}
				else
				{
					int priority = 48;
					
					if (buildingType.getID() == BWAPI::UnitTypes::Terran_Barracks.getID())
					{
						priority = 54;
					}
					
					actionQueue->push(
						new ConstructBuildingAction(priority, buildingTile, buildingType, worldModel));
				}
			}
			else
			{
				BWAPI::Broodwar->sendText("No free workers");
			}
		}

		//Monitor workers assigned to build something
		//If building has started (or worker was interrupted), unreserve resources
		std::vector<BWAPI::Unit*> keysToRemove = std::vector<BWAPI::Unit*>();

		for each (std::pair<BWAPI::Unit*, ConstructBuildingAction*> mapPair in worldModel->workersBuildingMap)
		{
			BWAPI::Unit* worker = mapPair.first;
			ConstructBuildingAction* action = mapPair.second;
			std::vector<BWAPI::Unit*>* buildings = worldModel->myUnitMap[action->buildingType];
			bool isBuildingStarted = false;

			if (buildings != NULL)
			{
				for each (BWAPI::Unit* building in (*buildings))
				{
					if (building->getTilePosition() == action->location)
					{
						isBuildingStarted = true;
					}
				}
			}

			if (worker == NULL
				|| ! worker->isConstructing()
				|| isBuildingStarted
				|| worker->isGatheringGas()
				|| worker->isGatheringMinerals())
			{
				worldModel->reservedMinerals -= action->buildingType.mineralPrice();
				worldModel->reservedGas -= action->buildingType.gasPrice();
				keysToRemove.push_back(worker);
			}
		}
		
		for each (BWAPI::Unit* key in keysToRemove)
		{
			delete worldModel->workersBuildingMap[key];
			worldModel->workersBuildingMap.erase(key);
		}
	}
	else
	{
		//Very beginning of the game
		actionQueue->push(new TrainUnitAction(50, cc, BWAPI::Broodwar->self()->getRace().getWorker()));
	}
}

MacroModule::~MacroModule(void)
{
}
