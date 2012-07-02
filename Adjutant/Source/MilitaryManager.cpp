#include "MilitaryManager.h"
#include "AdjutantAIModule.h"

MilitaryManager::MilitaryManager(void)
{
}

void MilitaryManager::evalute(ActionQueue* actionQueue)
{
	bool useModeling = AdjutantAIModule::useOpponentModeling;
	std::vector<UnitGroup*>* myArmyGroups = WorldManager::Instance().myArmyGroups;
	UnitGroup* baseGroup = myArmyGroups->front();

	if (WorldManager::Instance().isTerrainAnalyzed && BWAPI::Broodwar->getFrameCount() % 50 == 0)
	{
		//Init baseGroup location
		if (baseGroup->targetPosition == BWAPI::Position(0,0))
		{
			baseGroup->targetPosition = WorldManager::Instance().myHomeRegion->getCenter();
		}

		/*
		if (BWAPI::Broodwar->getFrameCount() % 100 == 0)
		{
			BWAPI::Broodwar->printf("MyArmy=%d () | EnemyArmy=%d ()",
				WorldManager::Instance().myArmyVector->size(),
				WorldManager::Instance().enemy->getUnits().size());
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
		if (WorldManager::Instance().enemy->getUnits().size() != 0)
		{
			static BWAPI::Position oldPosition = BWAPI::Position(0,0);

			//Mob Attack threat location
			if (BWAPI::Broodwar->getFrameCount() % 50 == 0)
			{
				armyPosition = (*WorldManager::Instance().enemy->getUnits().begin())->getPosition();
				oldPosition = (*WorldManager::Instance().enemy->getUnits().begin())->getPosition();
			}
			else
			{
				armyPosition = oldPosition;
			}
		}
		else if((!useModeling && WorldManager::Instance().enemyHomeRegion != NULL && WorldManager::Instance().myArmyVector->size() > 100)
			|| (useModeling && WorldManager::Instance().enemyHomeRegion != NULL && (WorldManager::Instance().getMyArmyValue() - WorldManager::Instance().getEnemyArmyValue() > 1000))
			)
		{
			static bool searchAndDestroy = false;
			static BWAPI::Position oldPosition = BWAPI::Position(0,0);

			if (myArmyGroups->at(1)->getCentroid().getDistance(WorldManager::Instance().enemyHomeRegion->getCenter()) < 200
				|| BWAPI::Broodwar->getFrameCount() > 50000)
			{
				searchAndDestroy = true;
			}

			if (searchAndDestroy)
			{
				if (BWAPI::Broodwar->getFrameCount() % 500 == 0)
				{
					if (WorldManager::Instance().enemyHistoricalUnitMap.size() > 0)
					{
						std::pair<int, HistoricalUnitInfo> firstPair = (*WorldManager::Instance().enemyHistoricalUnitMap.begin());
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
				armyPosition = WorldManager::Instance().enemyHomeRegion->getCenter();
			}
		}
		else
		{
			//TODO:account for multiple chokepoints
			BWTA::Region* homeRegion = WorldManager::Instance().myHomeRegion;
			
			if (homeRegion->getChokepoints().size() > 0)
			{
				//armyPosition = (*WorldManager::Instance().myHomeRegion->getChokepoints().begin())->getCenter();
				armyPosition = homeRegion->getCenter();
			}
			else
			{
				armyPosition = WorldManager::Instance().myHomeRegion->getCenter();
			}
		}

		myArmyGroups->at(1)->targetPosition = armyPosition;

		//Attack to location for all not near it
		for each (BWAPI::Unit* unit in (*myArmyGroups->at(1)->unitVector))
		{
			bool useLowLevelControl = false;
			BWAPI::Unit* closestEnemy = NULL;

			if (WorldManager::Instance().enemy->getUnits().size() != 0)
			{
				closestEnemy = Utils::getClosestUnit(unit, &WorldManager::Instance().enemy->getUnits());
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
		std::vector<BWAPI::Unit*> comsatVector = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Comsat_Station];
		std::vector<BWAPI::Unit*> sweepVector = WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Spell_Scanner_Sweep];

		if (! comsatVector.empty() && (sweepVector.empty() || sweepVector.size() == 0))
		{
			for each (BWAPI::Unit* enemyUnit in WorldManager::Instance().enemy->getUnits())
			{
				if (enemyUnit->isCloaked() || enemyUnit->isBurrowed() || enemyUnit->getType().hasPermanentCloak())
				{
					for each (BWAPI::Unit* comsat in comsatVector)
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

MilitaryManager::~MilitaryManager(void)
{
}
