#include "WorldModel.h"

WorldModel::WorldModel(void)
{
	//Initialize command center vector
	this->isTerrainAnalyzed = false;
	this->myArmyVector = new std::vector<BWAPI::Unit*>();
	this->myScoutVector = new std::vector<BWAPI::Unit*>();
	this->myWorkerVector = new std::vector<BWAPI::Unit*>();	

	for each (BWAPI::Player* player in BWAPI::Broodwar->getPlayers())
	{
		if ((! (player == BWAPI::Broodwar->self())) && player->getType() != BWAPI::PlayerTypes::Neutral)
		{
			this->enemy = player;
			break;
		}
	}
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
			if (this->myUnitMap.find(type) == this->myUnitMap.end())
			{
				this->myUnitMap[type] = new std::vector<BWAPI::Unit*>();
			}

			this->myUnitMap[type]->push_back(unit);
		}
		else if (Utils::unitIsEnemy(unit))
		{
			if (this->enemyUnitMap.find(type) == this->enemyUnitMap.end())
			{
				this->enemyUnitMap[type] = new std::vector<BWAPI::Unit*>();
			}

			this->enemyUnitMap[type]->push_back(unit);
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
					BWAPI::Broodwar->sendText("MyUnitComplete");
					if (unit->getType() == BWAPI::UnitTypes::Terran_SCV)
					{
						BWAPI::Broodwar->sendText("New Worker");
						this->myWorkerVector->push_back(unit);
					}
					break;
				
				case BWAPI::EventType::UnitDestroy:
					bool wasRemoved = false;

					wasRemoved = Utils::vectorRemoveElement(this->myWorkerVector, unit);
					if (!wasRemoved) {wasRemoved = Utils::vectorRemoveElement(this->myScoutVector, unit);}
					if (!wasRemoved) {wasRemoved = Utils::vectorRemoveElement(this->myArmyVector, unit);}

					if (wasRemoved)
					{
						BWAPI::Broodwar->sendText("MyUnitDestroy[T]");
					}
					else
					{
						BWAPI::Broodwar->sendText("MyUnitDestroy[F]");
					}
					break;
			}
		}
		else if (unit != NULL && Utils::unitIsEnemy(unit))
		{
			switch(gameEvent.getType())
			{
				case BWAPI::EventType::UnitDiscover:
					BWAPI::Broodwar->sendText("Enemy unit %s [%x] has been discovered at (%d,%d)",
						unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
					break;
				
				case BWAPI::EventType::UnitDestroy:
					BWAPI::Broodwar->sendText("Enemy unit %s [%x] has been destroyed at (%d,%d)",
						unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
					break;
			}
		}
	}

	if (isTerrainAnalyzed)
	{
		this->isTerrainAnalyzed = true;
		if (BWTA::getStartLocation(BWAPI::Broodwar->self()) != NULL)
		{
			this->myHomeRegion = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
		}
	}
}


WorldModel::~WorldModel(void)
{
	this->myUnitMap.clear();
	this->enemyUnitMap.clear();
	delete this->myWorkerVector;
	delete this->myScoutVector;
	delete this->myArmyVector;
}
