#include "AwarenessModule.h"
#include "AdjutantAIModule.h"

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

		if (AdjutantAIModule::useOpponentModeling
			&& BWAPI::Broodwar->getFrameCount() > 2000
			&& BWAPI::Broodwar->getFrameCount() % (2000 + (int)((1.0 - scoutingWeight) * 4000.0)) < 50
			&& worldModel->myScoutVector->empty())
		{
			//50% chance - Pick random base location to scout
			//50% chance - Pick enemy home base to scout
			std::set<BWTA::BaseLocation*> baseLocationSet = BWTA::getBaseLocations();
			
			unsigned int choice = rand() % (baseLocationSet.size() * 2);
			int count = 0;

			if (choice < baseLocationSet.size())
			{
				for each (BWTA::BaseLocation* base in baseLocationSet)
				{
					if (choice == count)
					{
						positionToExplore = base->getPosition();
						break;
					}
					count++;
				}
			}
			else if (worldModel->enemyHomeRegion != NULL)
			{
				positionToExplore = worldModel->enemyHomeRegion->getCenter();
			}
		}

		//Picking scouts
		if (worldModel->myScoutVector->empty() 
			&& workers->size() > 15 - (unsigned int)(10 * scoutingWeight)
			&& positionToExplore != BWAPI::Position(0,0))
		{
			BWAPI::Unit* firstScout = Utils::getFreeWorker(worldModel->myWorkerVector);

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
		}
		else
		{
			std::vector<BWAPI::Unit*> scoutsToRemove = std::vector<BWAPI::Unit*>();

			for each (BWAPI::Unit* scout in (*worldModel->myScoutVector))
			{
				//we know it's done
				if (scout->isIdle() || scout->isGatheringMinerals())
				{
					scoutsToRemove.push_back(scout);
				}				
			}

			for each (BWAPI::Unit* scout in scoutsToRemove)
			{
				actionQueue->push(new MoveAction(scout, worldModel->myHomeRegion->getCenter()));
				Utils::vectorRemoveElement(worldModel->myScoutVector, scout);
				worldModel->myWorkerVector->push_back(scout);
			}
		}

		//Capture enemy home base if we find it
		if (worldModel->enemyHomeRegion == NULL)
		{
			for each (BWAPI::Unit* enemyUnit in worldModel->enemy->getUnits())
			{
				if (enemyUnit->getType().isResourceDepot())
				{
					worldModel->enemyHomeRegion = BWAPI::Broodwar->getRegionAt(enemyUnit->getPosition());
				}
			}
		}

		//Control comsat use
		if (AdjutantAIModule::useOpponentModeling && worldModel->enemyHomeRegion != NULL)
		{
			std::vector<BWAPI::Unit*>* comsatVector = worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Comsat_Station];
			std::vector<BWAPI::Unit*>* sweepVector = worldModel->myUnitMap[BWAPI::UnitTypes::Spell_Scanner_Sweep];

			if (comsatVector != NULL && (sweepVector == NULL || sweepVector->size() == 0))
			{
				if (BWAPI::Broodwar->getFrameCount() % 10000 == 0)
				{
					for each (BWAPI::Unit* comsat in (*comsatVector))
					{
						if (comsat->getEnergy() > (BWAPI::TechTypes::Scanner_Sweep.energyUsed() * 3))
						{
							//TODO: Create use ability action
							comsat->useTech(BWAPI::TechTypes::Scanner_Sweep, worldModel->enemyHomeRegion->getCenter());
							break;
						}
					}
				}
			}
		}
	}
}

AwarenessModule::~AwarenessModule(void)
{
}
