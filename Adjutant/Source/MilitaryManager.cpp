#include "MilitaryManager.h"
#include "AdjutantAIModule.h"

MilitaryManager::MilitaryManager(void)
{
}

void MilitaryManager::evalute()
{
	Utils::log("Entering MilitaryManager", 1);

	if (WorldManager::Instance().isTerrainAnalyzed)
	{
		std::vector<UnitGroup*>* myArmyGroups = WorldManager::Instance().myArmyGroups;

		//For now, just have the whole army in group(1)
		if (myArmyGroups->size() < 2)
		{
			myArmyGroups->push_back(new UnitGroup());
		}

		UnitGroup* baseGroup = myArmyGroups->front();

		//Recruit worker if needed
		manageFightingWorkers();

		//Manage vultures as they go to lay mines
		manageVultureMining();

		//Init baseGroup location
		if (baseGroup->targetPosition == BWAPI::Positions::None)
		{
			if (WorldManager::Instance().myHomeBase != NULL)
			{
				baseGroup->targetPosition = WorldManager::Instance().myHomeBase->baseLocation->getRegion()->getCenter();
			}
		}

		if (baseGroup->size() > 0)
		{
			for each (BWAPI::Unit* unit in (*baseGroup->unitVector))
			{
				myArmyGroups->at(1)->addUnit(unit);
				unit->attack(myArmyGroups->at(1)->targetPosition);
			}

			baseGroup->removeAllUnits();
		}
		
		//Determine army behavior
		BWAPI::Position armyPosition = BWAPI::Positions::None;

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
		else if (WorldManager::Instance().enemyHomeRegion != NULL 
			&& (WorldManager::Instance().getMyArmyValue() - WorldManager::Instance().getEnemyArmyValue() > 1000))
		{
			static bool searchAndDestroy = false;
			static BWAPI::Position oldPosition = BWAPI::Position(0,0);

			if (myArmyGroups->at(1)->getCentroid().getDistance(WorldManager::Instance().enemyHomeRegion->getCenter()) < 400
				|| (BWAPI::Broodwar->getFrameCount() > 25000 && WorldManager::Instance().getMyArmyValue() > (WorldManager::Instance().getEnemyArmyValue() * 10))
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
		else if (WorldManager::Instance().myHomeBase != NULL)
		{
			//TODO:account for multiple chokepoints
			BWTA::Region* homeRegion = WorldManager::Instance().myHomeBase->baseLocation->getRegion();
			
			if (homeRegion->getChokepoints().size() > 0)
			{
				//armyPosition = (*WorldManager::Instance().myHomeRegion->getChokepoints().begin())->getCenter();
				armyPosition = homeRegion->getCenter();
			}
			else
			{
				armyPosition = homeRegion->getCenter();
			}
		}


		myArmyGroups->at(1)->targetPosition = armyPosition;
		std::set<BWAPI::Unit*> unitToRemoveSet = std::set<BWAPI::Unit*>();

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

			bool isUnitToRemove = false;
			manageUnitAbilities(unit, &isUnitToRemove);
			
			if (isUnitToRemove)
			{
				unitToRemoveSet.insert(unit);
			}

			//Need delay because constantly issuing attack command causes unit to do nothing
			if (BWAPI::Broodwar->getFrameCount() % 50 == 0 
				&& unit->getType() != BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
			{
				if (useLowLevelControl)
				{
					if (! unit->isAttacking())
					{
						if (closestEnemy->getDistance(unit) < unit->getType().groundWeapon().maxRange())
						{
							unit->attack(closestEnemy);
						}
						else
						{
							unit->attack(closestEnemy->getPosition());					
						}
					}
				}
				else if (unit->getDistance(myArmyGroups->at(1)->getCentroid()) > 500)
				{
					unit->attack(myArmyGroups->at(1)->getCentroid());
				}
				else if (unit->getDistance(myArmyGroups->at(1)->targetPosition) > 300)
				{
					unit->attack(myArmyGroups->at(1)->targetPosition);
				}
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
							comsat->useTech(BWAPI::TechTypes::Scanner_Sweep, enemyUnit->getPosition());
							break;
						}
					}
				}
			}
		}

		//Remove units from army that are under the control of other processes
		for each (UnitGroup* group in  (*WorldManager::Instance().myArmyGroups))
		{
			for each (BWAPI::Unit* unit in unitToRemoveSet)
			{
				group->removeUnit(unit);
			}
		}
	}

	Utils::log("Leaving MilitaryManager", 1);
}

void MilitaryManager::manageFightingWorkers()
{
	int totalProtectedThreat = 0;
	Threat* protectionThreat = NULL;

	for each (Threat* threat in WorldManager::Instance().threatVector)
	{
		if (threat->isInProtectedRegion())
		{
			protectionThreat = threat;
			totalProtectedThreat += threat->getAttackValue();
		}
	}

	while (totalProtectedThreat > WorldManager::getMyArmyValue() 
		&& WorldManager::Instance().myHomeBase->getTotalWorkerCount() > 0)
	{
		BWAPI::Unit* armyWorker = WorldManager::Instance().myHomeBase->
			removeWorkerNear(protectionThreat->getCentroid());
		
		if (armyWorker != NULL)
		{
			WorldManager::Instance().myArmyGroups->front()->addUnit(armyWorker);
		}
		else
		{
			break;
		}
	}

	if (totalProtectedThreat == 0)
	{
		for each (UnitGroup* group in  (*WorldManager::Instance().myArmyGroups))
		{
			group->removeType(BWAPI::UnitTypes::Terran_SCV);
		}
	}
}

void MilitaryManager::manageVultureMining()
{
	if (WorldManager::Instance().vulturesMiningMap.size() > 0
		&& BWAPI::Broodwar->getFrameCount() % (1 * Utils::FPS) == 0)
	{
		std::set<BWAPI::Unit*> unitToRemoveSet = std::set<BWAPI::Unit*>();

		for each (std::pair<BWAPI::Unit*, int> pair in WorldManager::Instance().vulturesMiningMap)
		{
			BWAPI::Unit* unit = pair.first;
			int distanceFromHomeToMine = pair.second;

			bool isEnemyClose = false;

			int curDist = unit->getDistance(WorldManager::Instance().myHomeBase->baseLocation->getPosition());
			
			std::set<BWAPI::Unit*> unitsInRange = unit->getUnitsInRadius(unit->getType().sightRange());

			for each (BWAPI::Unit* unit in unitsInRange)
			{
				if (Utils::unitIsEnemy(unit))
				{
					isEnemyClose = true;
					break;
				}
			}
			
			if (unit->isMoving() && 
				(curDist >= distanceFromHomeToMine
				|| isEnemyClose))
			{
				unit->useTech(BWAPI::TechTypes::Spider_Mines, unit->getPosition());
			}
			else if (unit->isIdle())
			{
				unitToRemoveSet.insert(unit);
			}
		}

		for each (BWAPI::Unit* unitToRemove in unitToRemoveSet)
		{
			WorldManager::Instance().vulturesMiningMap.erase(unitToRemove);
			WorldManager::Instance().myArmyGroups->front()->addUnit(unitToRemove);
		}
	}
}

void MilitaryManager::manageUnitAbilities(BWAPI::Unit* unit, bool* isUnitToRemove)
{
	BWAPI::UnitType type = unit->getType();

	//-------------------------------------------------------------------------
	// Marines and Firebats
	//-------------------------------------------------------------------------
	if (type == BWAPI::UnitTypes::Terran_Marine
		|| type == BWAPI::UnitTypes::Terran_Firebat)
	{
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Stim_Packs))
		{
			if (unit->isAttacking() && (! unit->isStimmed()))
			{
				unit->useTech(BWAPI::TechTypes::Stim_Packs);
			}
		}
	}
	//-------------------------------------------------------------------------
	// Vulture
	//-------------------------------------------------------------------------
	else if (type == BWAPI::UnitTypes::Terran_Vulture)
	{
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Spider_Mines)
			&& WorldManager::Instance().myHomeBase != NULL
			&& WorldManager::Instance().enemyHomeRegion != NULL
			&& BWAPI::Broodwar->getFrameCount() % (Utils::FPS * 5) == 0)
		{
			//TODO
			if (! unit->isAttacking() && unit->getSpiderMineCount() > 0)
			{
				BWAPI::Position homePos = WorldManager::Instance().myHomeBase->baseLocation->getPosition();
				BWAPI::Position enemyPos = WorldManager::Instance().enemyHomeRegion->getCenter();
				double maxDist = homePos.getDistance(enemyPos);
				
				WorldManager::Instance().vulturesMiningMap[unit] = rand() % (int)maxDist;
				unit->move(WorldManager::Instance().enemyHomeRegion->getCenter());
				*isUnitToRemove = true;
			}
		}
	}
	//-------------------------------------------------------------------------
	// Seige Tanks
	//-------------------------------------------------------------------------
	else if (type == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode
		|| type == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
	{
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode))
		{
			std::string currentOrder = unit->getOrder().getName();

			if( (currentOrder.compare(BWAPI::Orders::Unsieging.getName()) != 0) 
				&& (currentOrder.compare(BWAPI::Orders::Sieging.getName()) != 0) )
			{
				std::vector<BWAPI::Unit*> armyVector = *WorldManager::Instance().myArmyGroups->at(1)->unitVector;
				std::set<BWAPI::Unit*> myArmySet(armyVector.begin(), armyVector.end());

				BWAPI::Unit* closestEnemy = Utils::getClosestGroundUnit(unit, &WorldManager::Instance().enemy->getUnits());
				BWAPI::Unit* closestFriendly = Utils::getClosestUnitNotOfType(unit, &myArmySet);
				int enemyDistance = unit->getDistance(closestEnemy);
				int friendlyDistance = unit->getDistance(closestFriendly);
				
				if (unit->isSieged())
				{
					if ( enemyDistance > 300 && friendlyDistance > 100 )
					{
						unit->unsiege();
					}
				}
				else 
				{
					if ( enemyDistance <= 300 )  
					{
						unit->siege();
					}
				}
			}
		}
	}
	//-------------------------------------------------------------------------
	// Science Vessel
	//-------------------------------------------------------------------------
	else if (type == BWAPI::UnitTypes::Terran_Science_Vessel)
	{
		//TODO: BWAPI does not appear to expose range of friendly abilities
		const int DEFENSIVE_MATRIX_RANGE = 10 * 32;

		if (unit->getEnergy() >= BWAPI::TechTypes::Defensive_Matrix.energyUsed())
		{
			std::set<BWAPI::Unit*> unitsInRange = BWAPI::Broodwar->getUnitsInRadius(
				unit->getPosition(), 
				DEFENSIVE_MATRIX_RANGE);

			for each (BWAPI::Unit* targetUnit in unitsInRange)
			{
				if (targetUnit->isUnderAttack() && (! targetUnit->isDefenseMatrixed()))
				{
					unit->useTech(BWAPI::TechTypes::Defensive_Matrix, targetUnit);
					break;
				}
			}
		}
	}
}

MilitaryManager::~MilitaryManager(void)
{
}
