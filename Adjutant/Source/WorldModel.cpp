#include "WorldModel.h"

WorldModel::WorldModel(void)
{
	//Initialize command center vector
	this->isTerrainAnalyzed = false;
	this->myArmyVector = new std::vector<BWAPI::Unit*>();
	this->myScoutVector = new std::vector<BWAPI::Unit*>();
	this->myWorkerVector = new std::vector<BWAPI::Unit*>();	
}

void WorldModel::update(bool isTerrainAnalyzed)
{
	for each(BWAPI::Event gameEvent in BWAPI::Broodwar->getEvents())
	{
		BWAPI::Unit* unit = gameEvent.getUnit();
		
		switch(gameEvent.getType())
		{
			case BWAPI::EventType::UnitComplete:
				
				if (unit->getPlayer() == BWAPI::Broodwar->self())
				{
					handleOurUnitCreated(unit);
				}

				break;

			case BWAPI::EventType::UnitMorph:
				if (unit->getPlayer() == BWAPI::Broodwar->self())
				{
					handleOurUnitCreated(unit);
				}
				break;
			default:

				break;
		}
	}

	if (isTerrainAnalyzed)
	{
		this->isTerrainAnalyzed = true;
		if (BWTA::getStartLocation(BWAPI::Broodwar->self()) != NULL)
		{
			this->homeRegion = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
		}
	}
}


void WorldModel::handleOurUnitCreated(BWAPI::Unit* unit)
{
	BWAPI::UnitType type = unit->getType();

	if (this->myUnitMap.find(type) == this->myUnitMap.end())
	{
		this->myUnitMap[type] = new std::vector<BWAPI::Unit*>();
	}
	
	this->myUnitMap[type]->push_back(unit);

	if (type == BWAPI::UnitTypes::Terran_SCV)
	{
		this->myWorkerVector->push_back(unit);
	}
	else if (type == BWAPI::UnitTypes::Terran_Refinery)
	{
		//TODO:store refineries somehow so that we know when we have enough workers mining gas?
	}
}

WorldModel::~WorldModel(void)
{
	std::map<BWAPI::UnitType, std::vector<BWAPI::Unit*>*>::iterator iter;

	for (iter =  this->myUnitMap.begin(); iter != this->myUnitMap.end() ; iter++)
	{
		delete iter->second;
	}

	delete this->myWorkerVector;
	delete this->myScoutVector;
	delete this->myArmyVector;
}
