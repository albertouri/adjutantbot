#include "WorldManager.h"

WorldManager::WorldManager(void)
{
	//Initialize command center vector
	this->isTerrainAnalyzed = false;
	this->myArmyGroups = new std::vector<UnitGroup*>();
	this->myArmyVector = new std::vector<BWAPI::Unit*>();
	this->myScoutVector = new std::vector<BWAPI::Unit*>();
	this->myWorkerVector = new std::vector<BWAPI::Unit*>();	
	this->buildTaskVector = std::vector<BuildTask*>();

	this->myArmyGroups->push_back(new UnitGroup());

	for each (BWAPI::Player* player in BWAPI::Broodwar->getPlayers())
	{
		if ((! (player == BWAPI::Broodwar->self())) && player->getType() != BWAPI::PlayerTypes::Neutral)
		{
			this->enemy = player;
			break;
		}
	}
	
	this->myHomeBase = NULL;
	this->reservedMinerals = 0;
	this->reservedGas = 0;
}

void WorldManager::update(bool isTerrainAnalyzed)
{
	Utils::log("Entering WorldManager", 1);
	//Clear out previous structures so they can be refreshed
	this->myUnitMap.clear();
	this->enemyUnitMap.clear();
	this->imminentBuildingMap.clear();

	//Update unit maps based all visibile units
	for each (BWAPI::Unit* unit in BWAPI::Broodwar->getAllUnits())
	{
		BWAPI::UnitType type = unit->getType();
		
		if (Utils::unitIsMine(unit))
		{
			this->myUnitMap[type].push_back(unit);
		}
		else if (Utils::unitIsEnemy(unit))
		{
			this->enemyUnitMap[type].push_back(unit);
		}
	}

	//Use event checking to handle other groups
	for each(BWAPI::Event gameEvent in BWAPI::Broodwar->getEvents())
	{
		BWAPI::Unit* unit = gameEvent.getUnit();
		
		//My units
		if (unit != NULL && Utils::unitIsMine(unit))
		{
			switch(gameEvent.getType())
			{
				case BWAPI::EventType::UnitComplete:
					if (unit->getType() == BWAPI::UnitTypes::Terran_SCV)
					{
						this->myWorkerVector->push_back(unit);
					}
					else if (unit->getType().canMove())
					{
						this->myArmyVector->push_back(unit);

						//We always add units to the "0th" group - micro manager might split army using other groups
						//this->myArmyGroups->front()->addUnit(unit);
					}

					break;
				
				case BWAPI::EventType::UnitDestroy:
					Utils::vectorRemoveElement(this->myWorkerVector, unit);
					Utils::vectorRemoveElement(this->myScoutVector, unit);
					
					for each (UnitGroup* group in (*this->myArmyGroups))
					{
						group->removeUnit(unit);
					}
					break;
			}
		}
		else if (unit != NULL && Utils::unitIsEnemy(unit))
		{
			switch(gameEvent.getType())
			{
				case BWAPI::EventType::UnitDiscover:
					//BWAPI::Broodwar->sendText("Enemy unit %s [%x] has been discovered at (%d,%d)",
					//	unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
					if (this->enemyHistoricalUnitMap.find(unit->getID()) == this->enemyHistoricalUnitMap.end())
					{
						this->enemyHistoricalUnitMap[unit->getID()] = 
							HistoricalUnitInfo(unit->getID(), unit->getType(), unit->getPosition());
					}
					break;
				case BWAPI::EventType::UnitEvade:
					//BWAPI::Broodwar->sendText("Enemy unit %s [%x] has evaded at (%d,%d)",
					//	unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
					if (this->enemyHistoricalUnitMap.find(unit->getID()) != this->enemyHistoricalUnitMap.end())
					{
						this->enemyHistoricalUnitMap[unit->getID()].setPosition(unit->getPosition());
					}
					break;
				
				case BWAPI::EventType::UnitDestroy:
					//BWAPI::Broodwar->sendText("Enemy unit %s [%x] has been destroyed at (%d,%d)",
					//	unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
					if (this->enemyHistoricalUnitMap.find(unit->getID()) != this->enemyHistoricalUnitMap.end())
					{
						this->enemyHistoricalUnitMap.erase(unit->getID());
					}
					break;
			}
		}
	}

	//Build map of buildings that workers are on their way to build
	for each (std::pair<BWAPI::Unit*, BuildTask*> pair in this->workersBuildingMap)
	{
		imminentBuildingMap[pair.second->unitType]++;
	}
	
	if (isTerrainAnalyzed)
	{
		//Special check for initial base
		if (this->myHomeBase == NULL)
		{
			if (! this->myUnitMap[BWAPI::UnitTypes::Terran_Command_Center].empty())
			{
				this->addBase(this->myUnitMap[BWAPI::UnitTypes::Terran_Command_Center].front());
			}
		}

		//Check for new/destroyed bases and SCVs
		this->checkForBases();

		this->isTerrainAnalyzed = true;
	}

	Utils::log("Leaving WorldManager", 1);
}

void WorldManager::checkForBases()
{
	for each(BWAPI::Event e in BWAPI::Broodwar->getEvents())
	{
		BWAPI::Unit* unit = e.getUnit();
		
		if (unit != NULL && Utils::unitIsMine(unit))
		{
			switch(e.getType())
			{
				case BWAPI::EventType::UnitComplete:
				case BWAPI::EventType::UnitMorph:
					//Add new bases as they are created
					if (unit->getType().isResourceDepot())
					{
						this->addBase(unit);
					}
					else if (unit->getType() == BWAPI::UnitTypes::Terran_Refinery)
					{
						//Add new refineries as they are morphed
						for each (Base* base in this->myBaseVector)
						{
							if (base->addRefinery(unit))
							{
								break;
							}
						}
					}
					break;
				
				case BWAPI::EventType::UnitDestroy:
					//Remove bases as they are destroyed
					if (unit->getType().isResourceDepot())
					{
						Base* baseToRemove = NULL;

						for each (Base* base in this->myBaseVector)
						{
							if (base->resourceDepot == unit)
							{
								baseToRemove = base;
								break;
							}
						}

						if (baseToRemove != NULL)
						{
							//do not remove home base
							if (baseToRemove != this->myHomeBase)
							{
								Utils::vectorRemoveElement(&this->myBaseVector, baseToRemove);
								delete baseToRemove;
							}
						}
					}
					else if (unit->getType() == BWAPI::UnitTypes::Terran_Refinery)
					{
						//Remove refineries from bases as they are destroyed
						for each (Base* base in this->myBaseVector)
						{
							if (base->removeRefinery(unit))
							{
								break;
							}
						}
					}
					else if (unit->getType().isWorker())
					{
						//Remove workers from bases as they are destroyed
						for each (Base* base in this->myBaseVector)
						{
							if (base->removeWorker(unit))
							{
								break;
							}
						}
					}
					break;
			}
		}
	}
}

void WorldManager::addBase(BWAPI::Unit* commandCenter)
{
	//See if base already exists
	Base* base = NULL;

	for each (Base* b in this->myBaseVector)
	{
		if (b->baseLocation == BWTA::getNearestBaseLocation(commandCenter->getPosition()))
		{
			base = b;
		}
	}

	//Otherwise, Create new base
	if (base == NULL)
	{
		base = new Base(commandCenter);
	}

	BWTA::BaseLocation* location = base->baseLocation;

	//Check for any refineries that may already exist at the location
	for each (BWAPI::Unit* refinery in WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Refinery])
	{
		for each (BWAPI::Unit* geyser in location->getGeysers())
		{
			if (refinery->getPosition() == geyser->getPosition())
			{
				base->refineryVector.insert(refinery);
			}
		}
	}

	//Check for any SCVs that may already exist at the location
	for each (BWAPI::Unit* worker in WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_SCV])
	{
		if ((worker->isGatheringMinerals() && Utils::setContains(&location->getMinerals(), worker->getTarget()))
			|| (worker->isGatheringGas() && Utils::setContains(&base->refineryVector, worker->getTarget())))
		{
			base->addWorker(worker);
		}
	}

	if (! Utils::vectorContains(&WorldManager::Instance().myBaseVector, base))
	{
		WorldManager::Instance().myBaseVector.push_back(base);
		if (! Utils::vectorContains(&this->protectedRegionVector, base->baseLocation->getRegion()))
		{
			this->protectedRegionVector.push_back(base->baseLocation->getRegion());
		}
	}

	if (WorldManager::Instance().myHomeBase == NULL)
	{
		WorldManager::Instance().myHomeBase = base;
	}
}

int WorldManager::getEnemyArmyValue()
{
	int armyValue = 0;

	for each (std::pair<int, HistoricalUnitInfo> pair in this->enemyHistoricalUnitMap)
	{
		HistoricalUnitInfo hui = pair.second;

		if ( (hui.getType().canAttack() || hui.getType().isSpellcaster())
			&& ! hui.getType().isWorker())
		{
			armyValue += hui.getType().gasPrice() + hui.getType().mineralPrice();
		}
	}

	return armyValue;
}

int WorldManager::getMyArmyValue()
{
	int armyValue = 0;

	for each (UnitGroup* group in (*WorldManager::Instance().myArmyGroups))
	{
		for each (BWAPI::Unit* unit in (*group->unitVector))
		{
			armyValue += unit->getType().gasPrice() + unit->getType().mineralPrice();
		}
	}

	return armyValue;
}

int WorldManager::getMyAttackValue()
{
	int attackValue = 0;

	for each (UnitGroup* group in (*WorldManager::Instance().myArmyGroups))
	{
		for each (BWAPI::Unit* unit in (*group->unitVector))
		{
			if (unit->getType().isWorker())
			{
				attackValue += 13;
			}
			else
			{
				attackValue += unit->getType().gasPrice() + unit->getType().mineralPrice();
			}
		}
	}

	return attackValue;
}

double WorldManager::getEnemyRangedWeight()
{
	double actual = 0;
	double possible = 1;

	for each (std::pair<int, HistoricalUnitInfo> pair in this->enemyHistoricalUnitMap)
	{
		BWAPI::UnitType type = pair.second.getType();

		if (! type.isWorker() && ! type.isBuilding())
		{
			if (type.isFlyer())
			{
				actual += 1;
			}
			else if (type.groundWeapon().maxRange() 
						> BWAPI::UnitTypes::Zerg_Zergling.groundWeapon())
			{
				actual += 1;
			}

			possible += 1;
		}

		
	}

	return (actual / possible);
}

WorldManager::~WorldManager(void)
{
	this->myUnitMap.clear();
	this->enemyUnitMap.clear();
	this->imminentBuildingMap.clear();
	this->enemyHistoricalUnitMap.clear();
	this->workersBuildingMap.clear();
	delete this->myWorkerVector;
	delete this->myScoutVector;
	delete this->myArmyVector;

	//Deallocate each army group
	for each (UnitGroup* group in (*this->myArmyGroups))
	{
		delete group;
	}

	delete this->myArmyGroups;
}
