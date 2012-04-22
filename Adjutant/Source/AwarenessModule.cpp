#include "AwarenessModule.h"

AwarenessModule::AwarenessModule(void)
{
}

void AwarenessModule::evalute(WorldModel* worldModel, ActionQueue* actionQueue)
{
	double scoutingWeight = 0.75;
	std::vector<BWAPI::Unit*>* workers = worldModel->myWorkerVector;

	if (scoutingWeight > 0 && worldModel->isTerrainAnalyzed)
	{
		/*
		if (BWAPI::Broodwar->getFrameCount() % 100 == 0)
		{
			BWAPI::Broodwar->printf("Scouts=%d|workers=%d|vs=%d",
				worldModel->myScoutVector->size(),
				workers->size(),
				15 - (unsigned int)(10 * scoutingWeight));
		}
		*/
		
		//Picking locations of interest
		BWAPI::Position positionToExplore = BWAPI::Position(0,0);

		for each (BWTA::BaseLocation* base in BWTA::getStartLocations())
		{
			if (! BWAPI::Broodwar->isExplored(base->getTilePosition()))
			{
				positionToExplore = base->getPosition();
				break;
			}
		}

		//Picking scouts
		if (worldModel->myScoutVector->empty() 
			&& workers->size() > 15 - (unsigned int)(10 * scoutingWeight)
			&& positionToExplore != BWAPI::Position(0,0))
		{
			BWAPI::Unit* firstScout;

			for each (BWAPI::Unit* worker in (*workers))
			{
				if (! worker->isGatheringGas() && ! worker->isCarryingMinerals() && ! worker->isConstructing())
				{
					firstScout = worker;
					break;
				}
			}

			if (firstScout != NULL)
			{
				Utils::vectorRemoveElement(workers, firstScout);
				worldModel->myScoutVector->push_back(firstScout);
			}
		}

		//Issues explore or return commands
		if (! worldModel->myScoutVector->empty() && positionToExplore != BWAPI::Position(0,0))
		{
			for each (BWAPI::Unit* scout in (*worldModel->myScoutVector))
			{
				actionQueue->push(new MoveAction(scout, positionToExplore));
			}

			for each (BWAPI::Unit* enemyUnit in worldModel->enemy->getUnits())
			{
				if (enemyUnit->getType().isResourceDepot())
				{
					worldModel->enemyHomeRegion = BWAPI::Broodwar->getRegionAt(enemyUnit->getPosition());
				}
			}
		}
		else
		{
			//We've explored all bases, go back to mining
			for each (BWAPI::Unit* scout in (*worldModel->myScoutVector))
			{
				actionQueue->push(new MoveAction(scout, worldModel->myHomeRegion->getCenter()));
				worldModel->myWorkerVector->push_back(scout);
			}

			worldModel->myScoutVector->clear();
		}


	}
}

AwarenessModule::~AwarenessModule(void)
{
}
