#include "BuildManager.h"


BuildManager::BuildManager(void)
{
}

void BuildManager::evalute()
{
	BWAPI::Unit* cc = NULL;
	
	int gasWorkers = 0;
	int mineralWorkers = 0;

	//comsat
	if (BWAPI::Broodwar->getFrameCount() > 3000)
	{
		std::vector<BWAPI::Unit*> comandCenters = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Command_Center];

		if (! comandCenters.empty())
		{
			for each (BWAPI::Unit* cc in comandCenters)
			{
				if (cc->getAddon() == NULL)
				{
					cc->buildAddon(BWAPI::UnitTypes::Terran_Comsat_Station);
				}
			}
		}
	}

	std::vector<BuildTask*> tasksToRemove = std::vector<BuildTask*>();

	for each (BuildTask* buildTask in WorldManager::Instance().buildTaskVector)
	{
		if (buildTask->isConstructBuilding() && buildTask->position == BWAPI::TilePositions::Invalid)
		{
			//If position not defined, construct buildings randomly TODO: Make this not completely random
			BWAPI::UnitType buildingType = buildTask->unitType;
			bool isValidPosition = true;
			int counter = 0;

			BWAPI::Unit* workerPerformingBuild = Utils::getFreeWorker(WorldManager::Instance().myWorkerVector);
			
			if (workerPerformingBuild != NULL)
			{
				BWAPI::TilePosition buildingTile = WorldManager::Instance().myHomeBase->getTilePosition();

				do
				{
					//Start at command center + some random offset from the CC
					//gradually expand build radious
					buildingTile = WorldManager::Instance().myHomeBase->getTilePosition();
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
					//Reserve resources for SCV's travel to building location
					WorldManager::Instance().reservedGas += buildingType.gasPrice();
					WorldManager::Instance().reservedMinerals += buildingType.mineralPrice();
					WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new ConstructBuildingAction(buildTask->priority, 
						buildingTile, 
						buildingType);

					workerPerformingBuild->build(buildingTile, buildingType);

					BWAPI::Broodwar->printf("Worker at %d,%d sent to construct %s at %d,%d",
						workerPerformingBuild->getPosition().x(),
						workerPerformingBuild->getPosition().y(),
						buildingType.getName().c_str(),
						buildingTile.x(),
						buildingTile.y());

					tasksToRemove.push_back(buildTask);
				}
			}
			else
			{
				BWAPI::Broodwar->sendText("No free workers");
			}
		}
		else if (buildTask->isConstructBuilding() && buildTask->position != BWAPI::TilePositions::Invalid)
		{
			BWAPI::Unit* workerPerformingBuild = Utils::getFreeWorker(WorldManager::Instance().myWorkerVector);
			BWAPI::UnitType buildingType = buildTask->unitType;

			if (workerPerformingBuild != NULL)
			{
				//Reserve resources for SCV's travel to building location
				WorldManager::Instance().reservedGas += buildingType.gasPrice();
				WorldManager::Instance().reservedMinerals += buildingType.mineralPrice();
				WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new ConstructBuildingAction(buildTask->priority, 
					buildTask->position, 
					buildingType);

				workerPerformingBuild->build(buildTask->position, buildingType);

				BWAPI::Broodwar->printf("Worker at %d,%d sent to construct %s at %d,%d",
					workerPerformingBuild->getPosition().x(),
					workerPerformingBuild->getPosition().y(),
					buildingType.getName().c_str(),
					buildTask->position.x(),
					buildTask->position.y());

				tasksToRemove.push_back(buildTask);
			}
		}
		else if (buildTask->isTrainUnit())
		{
			buildTask->buildingToUse->train(buildTask->unitType);
			tasksToRemove.push_back(buildTask);
		}
	}

	for each (BuildTask* task in tasksToRemove)
	{
		Utils::vectorRemoveElement(&WorldManager::Instance().buildTaskVector, task);
		delete task;
	}

	//Monitor workers assigned to build something
	//If building has started (or worker was interrupted), unreserve resources
	std::vector<BWAPI::Unit*> keysToRemove = std::vector<BWAPI::Unit*>();

	for each (std::pair<BWAPI::Unit*, ConstructBuildingAction*> mapPair in WorldManager::Instance().workersBuildingMap)
	{
		BWAPI::Unit* worker = mapPair.first;
		ConstructBuildingAction* action = mapPair.second;
		std::vector<BWAPI::Unit*> buildings = WorldManager::Instance().myUnitMap[action->buildingType];
		bool isBuildingStarted = false;

		if (! buildings.empty())
		{
			for each (BWAPI::Unit* building in buildings)
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
			WorldManager::Instance().reservedMinerals -= action->buildingType.mineralPrice();
			WorldManager::Instance().reservedGas -= action->buildingType.gasPrice();
			keysToRemove.push_back(worker);
		}
	}
	
	for each (BWAPI::Unit* key in keysToRemove)
	{
		delete WorldManager::Instance().workersBuildingMap[key];
		WorldManager::Instance().workersBuildingMap.erase(key);
	}
}

BuildManager::~BuildManager(void)
{
}