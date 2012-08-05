#include "MicroModule.h"
#include "AdjutantAIModule.h"

MicroModule::MicroModule(void)
{
}

void MicroModule::evalute(WorldModel* worldModel, ActionQueue* actionQueue)
{
	bool useModeling = AdjutantAIModule::useOpponentModeling;
	std::vector<UnitGroup*>* myArmyGroups = worldModel->myArmyGroups;
	UnitGroup* baseGroup = myArmyGroups->front();

	if (worldModel->isTerrainAnalyzed && BWAPI::Broodwar->getFrameCount() % 50 == 0)
	{
		//Init baseGroup location
		if (baseGroup->targetPosition == BWAPI::Position(0,0))
		{
			baseGroup->targetPosition = worldModel->myHomeRegion->getCenter();
		}

		/*
		if (BWAPI::Broodwar->getFrameCount() % 100 == 0)
		{
			BWAPI::Broodwar->printf("MyArmy=%d () | EnemyArmy=%d ()",
				worldModel->myArmyVector->size(),
				worldModel->enemy->getUnits().size());
		}
		*/

		//For now, just have the whole army in group(1)
		if (myArmyGroups->size() < 2)
		{
			myArmyGroups->push_back(new UnitGroup());
		}

		if (baseGroup->size() > 0)
		{
			for each (BWAPI::Unit* unit in (*baseGroup->unitVector))
			{
				myArmyGroups->at(1)->addUnit(unit);
				actionQueue->push(new AttackAction(
					unit, 
					myArmyGroups->at(1)->targetPosition
					));
			}

			baseGroup->removeAllUnits();
		}
		
		//Determine army behavior
		BWAPI::Position armyPosition = BWAPI::Position(0,0);

		//Threats
		if (worldModel->enemy->getUnits().size() != 0)
		{
			static BWAPI::Position oldPosition = BWAPI::Position(0,0);

			//Mob Attack threat location
			if (BWAPI::Broodwar->getFrameCount() % 50 == 0)
			{
				armyPosition = (*worldModel->enemy->getUnits().begin())->getPosition();
				oldPosition = (*worldModel->enemy->getUnits().begin())->getPosition();
			}
			else
			{
				armyPosition = oldPosition;
			}
		}
		else if((!useModeling && worldModel->enemyHomeRegion != NULL && worldModel->myArmyVector->size() > 100)
			|| (useModeling && worldModel->enemyHomeRegion != NULL && (worldModel->getMyArmyValue() - worldModel->getEnemyArmyValue() > 1000))
			)
		{
			static bool searchAndDestroy = false;
			static BWAPI::Position oldPosition = BWAPI::Position(0,0);

			if (myArmyGroups->at(1)->getCentroid().getDistance(worldModel->enemyHomeRegion->getCenter()) < 200
				|| BWAPI::Broodwar->getFrameCount() > 50000)
			{
				searchAndDestroy = true;
			}

			if (searchAndDestroy)
			{
				if (BWAPI::Broodwar->getFrameCount() % 500 == 0)
				{
					if (worldModel->enemyHistoricalUnitMap.size() > 0)
					{
						std::pair<int, HistoricalUnitInfo> firstPair = (*worldModel->enemyHistoricalUnitMap.begin());
						armyPosition = BWAPI::Position(firstPair.second.getPosition());
					}
					else
					{
						int randTileX = rand() % BWAPI::Broodwar->mapWidth();
						int randTileY = rand() % BWAPI::Broodwar->mapHeight();

						armyPosition = BWAPI::Position(BWAPI::TilePosition(randTileX, randTileY));
					}
					oldPosition = armyPosition;
				}
				else
				{
					armyPosition = oldPosition;
				}
			}
			else
			{
				armyPosition = worldModel->enemyHomeRegion->getCenter();
			}
		}
		else
		{
			//TODO:account for multiple chokepoints
			BWTA::Region* homeRegion = worldModel->myHomeRegion;
			
			if (homeRegion->getChokepoints().size() > 0)
			{
				//armyPosition = (*worldModel->myHomeRegion->getChokepoints().begin())->getCenter();
				armyPosition = homeRegion->getCenter();
			}
			else
			{
				armyPosition = worldModel->myHomeRegion->getCenter();
			}
		}

		myArmyGroups->at(1)->targetPosition = armyPosition;

		//Attack to location for all not near it
		for each (BWAPI::Unit* unit in (*myArmyGroups->at(1)->unitVector))
		{
			bool useLowLevelControl = false;
			BWAPI::Unit* closestEnemy = NULL;

			if (worldModel->enemy->getUnits().size() != 0)
			{
				closestEnemy = Utils::getClosestUnit(unit, &worldModel->enemy->getUnits());
				if (closestEnemy->getDistance(unit) < 500)
				{
					useLowLevelControl = true;
				}	
			}

			if (useLowLevelControl)
			{
				if (unit->isIdle())
				{
					if (closestEnemy->getDistance(unit) < unit->getType().groundWeapon().maxRange())
					{
					actionQueue->push(new AttackAction(
						unit, 
						closestEnemy
						));
					}
					else
					{
						actionQueue->push(new AttackAction(
							unit, 
							closestEnemy->getPosition()
							));
					}
				}
			}
			else if (unit->getDistance(myArmyGroups->at(1)->getCentroid()) > 500)
			{
				actionQueue->push(new AttackAction(
					unit, 
					myArmyGroups->at(1)->getCentroid()
					));
			}
			else if (unit->getDistance(myArmyGroups->at(1)->targetPosition) > 300)
			{
				actionQueue->push(new AttackAction(
					unit, 
					myArmyGroups->at(1)->targetPosition
					));
			}
		}

		//Control comsat use
		std::vector<BWAPI::Unit*>* comsatVector = worldModel->myUnitMap[BWAPI::UnitTypes::Terran_Comsat_Station];
		std::vector<BWAPI::Unit*>* sweepVector = worldModel->myUnitMap[BWAPI::UnitTypes::Spell_Scanner_Sweep];

		if (comsatVector != NULL && (sweepVector == NULL || sweepVector->size() == 0))
		{
			for each (BWAPI::Unit* enemyUnit in worldModel->enemy->getUnits())
			{
				if (enemyUnit->isCloaked() || enemyUnit->isBurrowed() || enemyUnit->getType().hasPermanentCloak())
				{
					for each (BWAPI::Unit* comsat in (*comsatVector))
					{
						if (comsat->getEnergy() > BWAPI::TechTypes::Scanner_Sweep.energyUsed())
						{
							//TODO: Create use ability action
							comsat->useTech(BWAPI::TechTypes::Scanner_Sweep, enemyUnit->getPosition());
							break;
						}
					}
				}
			}
		}
	}
}

MicroModule::~MicroModule(void)
{
}
