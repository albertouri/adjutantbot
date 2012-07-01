#include "WorldModel.h"

WorldModel::WorldModel(void)
{
	//Initialize command center vector
	this->isTerrainAnalyzed = false;
	this->myArmyGroups = new std::vector<UnitGroup*>();
	this->myArmyVector = new std::vector<BWAPI::Unit*>();
	this->myScoutVector = new std::vector<BWAPI::Unit*>();
	this->myWorkerVector = new std::vector<BWAPI::Unit*>();	

	this->myArmyGroups->push_back(new UnitGroup());

	for each (BWAPI::Player* player in BWAPI::Broodwar->getPlayers())
	{
		if ((! (player == BWAPI::Broodwar->self())) && player->getType() != BWAPI::PlayerTypes::Neutral)
		{
			this->enemy = player;
			break;
		}
	}
	
	this->reservedMinerals = 0;
	this->reservedGas = 0;
}

void WorldModel::update(bool isTerrainAnalyzed)
{
	//Clear out previous structures so they can be refreshed
	this->myUnitMap.clear();
	this->enemyUnitMap.clear();

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
						this->myArmyGroups->front()->addUnit(unit);
					}

					break;
				
				case BWAPI::EventType::UnitDestroy:
					bool wasRemoved = false;

					wasRemoved = Utils::vectorRemoveElement(this->myWorkerVector, unit);
					if (!wasRemoved) {wasRemoved = Utils::vectorRemoveElement(this->myScoutVector, unit);}
					if (!wasRemoved) {wasRemoved = Utils::vectorRemoveElement(this->myArmyVector, unit);}
					
					for each (UnitGroup* group in (*this->myArmyGroups))
					{
						if (group->removeUnit(unit)) {break;}
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

	if (isTerrainAnalyzed)
	{
		if (BWTA::getStartLocation(BWAPI::Broodwar->self()) != NULL)
		{
			this->myHomeRegion = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
			this->myHomeBase = BWTA::getNearestBaseLocation(this->myHomeRegion->getCenter());
		}
		this->isTerrainAnalyzed = true;
	}
}

int WorldModel::getEnemyArmyValue()
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

int WorldModel::getMyArmyValue()
{
	int armyValue = 0;

	for each (BWAPI::Unit* unit in (*this->myArmyVector))
	{
		armyValue += unit->getType().gasPrice() + unit->getType().mineralPrice();
	}

	return armyValue;
}

double WorldModel::getEnemyRangedWeight()
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

WorldModel::~WorldModel(void)
{
	this->myUnitMap.clear();
	this->enemyUnitMap.clear();
	this->enemyHistoricalUnitMap.clear();
	this->workersBuildingMap.clear();
	this->enemyHistoricalUnitMap.clear();
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
