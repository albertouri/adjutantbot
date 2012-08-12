#include "BuildManager.h"

BuildManager::BuildManager(void)
{
	this->reservedMap->getInstance()->create();
    this->defaultBuildingPlacer = new BFSBuildingPlacer();
	this->showDebugInfo = false;
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
		// Construct Building 
		if (buildTask->isConstructBuilding())
		{
			bool isSuccess = false;
			BWAPI::UnitType buildingType = buildTask->unitType;

			if (buildingType.isAddon())
			{
				//Add-on
				BWAPI::Unit* buildingToUse = buildTask->buildingToUse;

				if (buildingToUse == NULL)
				{
					std::vector<BWAPI::Unit*> builtByVector = 
						WorldManager::Instance().myUnitMap[buildingType.whatBuilds().first];

					for each (BWAPI::Unit* building in builtByVector)
					{
						if (building->getAddon() == NULL && Utils::isBuildingReady(building))
						{
							buildingToUse = building;
							break;
						}
					}
				}

				if (buildingToUse != NULL)
				{
					if (buildTask->frameStarted == -1) 
					{ 
						buildTask->frameStarted = BWAPI::Broodwar->getFrameCount();
					}

					if (buildingToUse->buildAddon(buildingType))
					{
						isSuccess = true;
						tasksToRemove.push_back(buildTask);
						break;
					}
				}
			}
			else
			{
				//Normal building
				BWAPI::Unit* workerPerformingBuild = 
					Utils::getFreeWorker(WorldManager::Instance().myWorkerVector);

				if (workerPerformingBuild != NULL)
				{
					BWAPI::TilePosition buildLocation = buildTask->position;

					if (buildLocation == BWAPI::TilePositions::Invalid)
					{
						BWAPI::TilePosition seedTile = WorldManager::Instance().myHomeBase->baseLocation->getTilePosition();

						buildLocation = this->defaultBuildingPlacer->findBuildLocation(
							this->reservedMap->getInstance(), buildingType, 
							seedTile, workerPerformingBuild);
					}

					if (buildLocation != BWAPI::TilePositions::Invalid)
					{
						if (buildTask->frameStarted == -1) 
						{ 
							buildTask->frameStarted = BWAPI::Broodwar->getFrameCount();
						}

						if (workerPerformingBuild->build(buildLocation, buildingType))
						{
							//Reserve resources for SCV's travel to building location
							WorldManager::Instance().reservedGas += buildingType.gasPrice();
							WorldManager::Instance().reservedMinerals += buildingType.mineralPrice();
							WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new BuildTask(buildTask->priority, 
								buildingType,
								buildLocation);
							WorldManager::Instance().workersBuildingMap[workerPerformingBuild]->frameStarted = BWAPI::Broodwar->getFrameCount();

							if (showDebugInfo)
							{
								BWAPI::Broodwar->printf("Worker at %d,%d sent to construct %s at %d,%d",
									workerPerformingBuild->getPosition().x(),
									workerPerformingBuild->getPosition().y(),
									buildingType.getName().c_str(),
									buildLocation.x(),
									buildLocation.y());
							}

							isSuccess = true;
							tasksToRemove.push_back(buildTask);
						}
					}
				}
			}

			//If it's been 2 minutes, give up
			if (BWAPI::Broodwar->getFrameCount() - buildTask->frameStarted > (Utils::FPS * 120)
				&& ! isSuccess)
			{
				tasksToRemove.push_back(buildTask);
			}
		}
		// Train Unit
		else if (buildTask->isTrainUnit())
		{
			BWAPI::UnitType unitToBuild = buildTask->unitType;
			BWAPI::Unit* buildingToUse = buildTask->buildingToUse;

			// Pre-Defined Training site to use
			if(buildingToUse == NULL )
			{
				BWAPI::UnitType builtBy = unitToBuild.whatBuilds().first;

				for each (BWAPI::Unit* building in WorldManager::Instance().myUnitMap[builtBy])
				{
					if (Utils::isBuildingReady(building))
					{
						buildingToUse = building;
						break;
					}
				}
			}

			if (buildingToUse != NULL && buildingToUse->train(unitToBuild))
			{
				if (showDebugInfo)
				{				
					BWAPI::Broodwar->printf("Training Unit %s at %s",
						unitToBuild.getName().c_str(), 
						buildingToUse->getType().getName().c_str());
				}

				tasksToRemove.push_back(buildTask);
			}
		}
		// Upgrade
		else if (buildTask->isUpgrade())	
		{
			BWAPI::UpgradeType upgradeType = buildTask->upgradeType;
			int currentLevel = BWAPI::Broodwar->self()->getUpgradeLevel(upgradeType);
			BWAPI::UnitType upgradedBy = buildTask->upgradeType.whatUpgrades();
			BWAPI::UnitType requiredUnit = buildTask->upgradeType.whatsRequired(currentLevel + 1);
			int requiredCount = BWAPI::Broodwar->self()->completedUnitCount(requiredUnit);

			if (requiredUnit == BWAPI::UnitTypes::None || requiredCount > 0)
			{
				for each (BWAPI::Unit* building in WorldManager::Instance().myUnitMap[upgradedBy])
				{
					if (Utils::isBuildingReady(building))
					{
						if (showDebugInfo)
						{
							BWAPI::Broodwar->printf("Upgrading %s",
								upgradeType.c_str());
						}

						building->upgrade(upgradeType);
						tasksToRemove.push_back(buildTask);
						break;
					}
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
				for each (BWAPI::Unit* building in units)
				{
					if (Utils::isBuildingReady(building))
					{
						if (showDebugInfo)
						{
							BWAPI::Broodwar->printf("Researching Tech %s",
									techType.c_str());
						}

						building->research(techType);
						tasksToRemove.push_back(buildTask);
						break;
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
			|| (BWAPI::Broodwar->getFrameCount() - task->frameStarted > (Utils::FPS * 5) &&
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