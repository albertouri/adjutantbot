#include "BuildManager.h"

BuildManager::BuildManager(void)
{
	this->reservedMap->getInstance()->create();
    this->defaultBuildingPlacer = new BFSBuildingPlacer();
}

void BuildManager::evalute()
{
	/**
	* Possible move into UnitManager or WorldManager as it could be a proformance hang later
	*/
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
		// Building 
		if (buildTask->isConstructBuilding())
		{
			BWAPI::TilePosition buildLocationTile = WorldManager::Instance().myHomeBase->baseLocation->getTilePosition();
			BuildingPlacer* placer = defaultBuildingPlacer;

			BWAPI::UnitType buildingType = buildTask->unitType;
			BWAPI::Unit* workerPerformingBuild = 
					Utils::getFreeWorker(WorldManager::Instance().myWorkerVector);

			if (workerPerformingBuild != NULL)
			{
				// Pre-Defined Building Site Location to use
				if (buildTask->position != BWAPI::TilePositions::Invalid)
				{
					//Reserve resources for SCV's travel to building location
					WorldManager::Instance().reservedGas += buildingType.gasPrice();
					WorldManager::Instance().reservedMinerals += buildingType.mineralPrice();
					WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new BuildTask(buildTask->priority, 
						buildingType,
						buildTask->position);
					WorldManager::Instance().workersBuildingMap[workerPerformingBuild]->frameExecuted = BWAPI::Broodwar->getFrameCount();

					workerPerformingBuild->build(buildTask->position, buildingType);

					BWAPI::Broodwar->printf("Worker at %d,%d sent to construct %s at %d,%d",
						workerPerformingBuild->getPosition().x(),
						workerPerformingBuild->getPosition().y(),
						buildingType.getName().c_str(),
						buildTask->position.x(),
						buildTask->position.y());

					tasksToRemove.push_back(buildTask);

				}
				// No Pre-Defined Building Site Location to use
				else
				{
					BWAPI::TilePosition buildLocation = placer->findBuildLocation(this->reservedMap->getInstance(), buildingType, 
						buildLocationTile, workerPerformingBuild);
			
					if ( buildLocation != BWAPI::TilePositions::None )
					{
						//t->setBuildLocation( buildLocation );
						this->reservedMap->getInstance()->reserveTiles(buildLocation, buildingType);

						//Reserve resources for SCV's travel to building location
						WorldManager::Instance().reservedGas += buildingType.gasPrice();
						WorldManager::Instance().reservedMinerals += buildingType.mineralPrice();
						WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new BuildTask(buildTask->priority, 
							buildingType, 
							buildLocation);
						WorldManager::Instance().workersBuildingMap[workerPerformingBuild]->frameExecuted = BWAPI::Broodwar->getFrameCount();

						workerPerformingBuild->build(buildLocation, buildingType);

						BWAPI::Broodwar->printf("Worker at %d,%d sent to construct %s at %d,%d",
							workerPerformingBuild->getPosition().x(),
							workerPerformingBuild->getPosition().y(),
							buildingType.getName().c_str(),
							buildLocation.x(),
							buildLocation.y());

						tasksToRemove.push_back(buildTask);
					}
					else
					{
						BWAPI::Broodwar->sendText("Unable to find building Location");
					}
				}
			}
			else
			{
				BWAPI::Broodwar->sendText("No free workers");
			}
		}
		// Train Unit
		else if (buildTask->isTrainUnit())
		{
			BWAPI::UnitType trainingType = buildTask->unitType;
	
			// Pre-Defined Training site to use
			if( buildTask->buildingToUse != NULL )
			{
				buildTask->buildingToUse->train(trainingType);
				BWAPI::Broodwar->printf("Training Unit %s at %s",
					trainingType.getName().c_str(), 
					buildTask->buildingToUse->getType().getName().c_str());

				tasksToRemove.push_back(buildTask);
			}
			// No Pre-Defined Training site to use
			else
			{
				BWAPI::UnitType trainingSiteType = trainingType.whatBuilds().first;

				std::vector<BWAPI::Unit*> trainingSites = WorldManager::Instance().myUnitMap[trainingSiteType];
				if (!trainingSites.empty() && trainingSites.size() > 0)
				{
					// TODO look into what happens if trainSite is fully queued
					// might need to ensure training has a site to train at
					trainingSites.front()->train(trainingType);
					BWAPI::Broodwar->printf("Training Unit %s at %s",
						trainingType.getName().c_str(), 
						trainingSiteType.getName().c_str());

					tasksToRemove.push_back(buildTask);
				}
			}
		}
		// Upgrade
		else if (buildTask->isUpgrade())	
		{
			BWAPI::UpgradeType upgradeType = buildTask->upgradeType;
			BWAPI::UnitType requiredUnitType = upgradeType.whatsRequired();

			std::vector<BWAPI::Unit*> requiredUnits = WorldManager::Instance().myUnitMap[requiredUnitType];
			if (!requiredUnits.empty() && requiredUnits.size() > 0)
			{
				BWAPI::UnitType upgradeUnitType = upgradeType.whatUpgrades();

				std::vector<BWAPI::Unit*> upgradeUnits = WorldManager::Instance().myUnitMap[upgradeUnitType];
				if (!upgradeUnits.empty() && upgradeUnits.size() > 0)
				{
					upgradeUnits.front()->upgrade(upgradeType);
					BWAPI::Broodwar->printf("Creating Upgrade %s",
						upgradeType.c_str());

					tasksToRemove.push_back(buildTask);
				}
			}
		}
		// Tech Advancement
		else if (buildTask->isTech())
		{
			BWAPI::TechType techType = buildTask->techType;
			BWAPI::UnitType unitType = techType.whatResearches();

			std::vector<BWAPI::Unit*> units = WorldManager::Instance().myUnitMap[unitType];
			if (!units.empty() && units.size() > 0)
			{
				units.front()->research(techType);
				BWAPI::Broodwar->printf("Researching Tech %s",
						techType.c_str());

				tasksToRemove.push_back(buildTask);
			}
		}
		// Add-on
		else if(buildTask->unitType.isAddon())
		{
			BWAPI::UnitType addonType = buildTask->unitType;
			BWAPI::Unit* workerPerformingBuild = 
					Utils::getFreeWorker(WorldManager::Instance().myWorkerVector);

			if (workerPerformingBuild != NULL)
			{
				// Pre-Defined Building site to use for the Addon
				if( buildTask->buildingToUse != NULL )
				{
					BWAPI::TilePosition buildUnitLocation = buildTask->buildingToUse->getTilePosition();

					//Reserve resources for SCV's travel to building location
					WorldManager::Instance().reservedGas += addonType.gasPrice();
					WorldManager::Instance().reservedMinerals += addonType.mineralPrice();
					WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new BuildTask(buildTask->priority, 
						addonType, 
						buildUnitLocation);
					WorldManager::Instance().workersBuildingMap[workerPerformingBuild]->frameExecuted = BWAPI::Broodwar->getFrameCount();

					BWAPI::Broodwar->printf("Worker at %d,%d sent to construct Addon %s at %d,%d",
						workerPerformingBuild->getPosition().x(),
						workerPerformingBuild->getPosition().y(),
						addonType.getName().c_str(),
						buildUnitLocation.x(),
						buildUnitLocation.y());

					workerPerformingBuild->buildAddon(addonType);

					tasksToRemove.push_back(buildTask);
				}
				// No Pre-Defined Building site to use for the Addon
				else
				{
					BWAPI::UnitType buildingToUse = addonType.whatBuilds().first;

					std::vector<BWAPI::Unit*> availableBuildingToUse = WorldManager::Instance().myUnitMap[buildingToUse];
					if (!availableBuildingToUse.empty() && availableBuildingToUse.size() > 0)
					{
						BWAPI::TilePosition buildUnitLocation = availableBuildingToUse.front()->getTilePosition();

						//Reserve resources for SCV's travel to building location
						WorldManager::Instance().reservedGas += addonType.gasPrice();
						WorldManager::Instance().reservedMinerals += addonType.mineralPrice();
						WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new BuildTask(buildTask->priority, 
							addonType, 
							buildUnitLocation);
						WorldManager::Instance().workersBuildingMap[workerPerformingBuild]->frameExecuted = BWAPI::Broodwar->getFrameCount();

						BWAPI::Broodwar->printf("Worker at %d,%d sent to construct Addon %s at %d,%d",
							workerPerformingBuild->getPosition().x(),
							workerPerformingBuild->getPosition().y(),
							addonType.getName().c_str(),
							buildUnitLocation.x(),
							buildUnitLocation.y());

						workerPerformingBuild->buildAddon(addonType);

						tasksToRemove.push_back(buildTask);
					}
				}
			}
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

	for each (std::pair<BWAPI::Unit*, BuildTask*> mapPair in WorldManager::Instance().workersBuildingMap)
	{
		BWAPI::Unit* worker = mapPair.first;
		BuildTask* task = mapPair.second;
		std::vector<BWAPI::Unit*> buildings = WorldManager::Instance().myUnitMap[task->unitType];
		bool isBuildingStarted = false;

		if (! buildings.empty())
		{
			for each (BWAPI::Unit* building in buildings)
			{
				if (building->getTilePosition() == task->position)
				{
					isBuildingStarted = true;
				}
			}
		}

		if (! worker->exists()
			|| isBuildingStarted
			|| (BWAPI::Broodwar->getFrameCount() - task->frameExecuted > (23*5) &&
				(! worker->isConstructing()
				|| worker->isGatheringGas()
				|| worker->isGatheringMinerals()
				))
			)
		{
			WorldManager::Instance().reservedMinerals -= task->unitType.mineralPrice();
			WorldManager::Instance().reservedGas -= task->unitType.gasPrice();
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
	this->reservedMap->getInstance()->destroy();
	delete this->defaultBuildingPlacer;
}