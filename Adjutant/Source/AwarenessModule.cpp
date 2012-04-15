#include "AwarenessModule.h"

AwarenessModule::AwarenessModule(void)
{
}

void AwarenessModule::evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue)
{
	double scoutingWeight = 0.75;
	std::vector<BWAPI::Unit*>* workers = worldModel->myWorkerVector;

	if (scoutingWeight > 0 && worldModel->isTerrainAnalyzed)
	{
		
		if (BWAPI::Broodwar->getFrameCount() % 200 == 0)
		{
			BWAPI::Broodwar->printf("Scouts=%d|workers=%d|vs=%d",
				worldModel->myScoutVector->size(),
				workers->size(),
				15 - (unsigned int)(10 * scoutingWeight));
		}

		//Pick initial scout
		if (worldModel->myScoutVector->empty() 
			&& workers->size() > 15 - (unsigned int)(10 * scoutingWeight))
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

		if (! worldModel->myScoutVector->empty())
		{
			BWAPI::Position positionToExplore;

			for each (BWTA::BaseLocation* base in BWTA::getStartLocations())
			{
				if (! BWAPI::Broodwar->isExplored(base->getTilePosition()))
				{
					positionToExplore = base->getPosition();
					break;
				}
			}

			for each (BWAPI::Unit* scout in (*worldModel->myScoutVector))
			{
				actionQueue->push(new MoveAction(scout, positionToExplore));
			}
		}
	}
}

AwarenessModule::~AwarenessModule(void)
{
}
