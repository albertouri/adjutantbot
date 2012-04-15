#include "WorldModel.h"

WorldModel::WorldModel(void)
{
	//Initialize command center vector
	this->myWorkerVector = new std::vector<BWAPI::Unit*>();
	this->myArmyVector = new std::vector<BWAPI::Unit*>();
}

void WorldModel::update()
{
	std::set<int> test;
	std::vector<int> test2;
	
	for each(BWAPI::Event gameEvent in BWAPI::Broodwar->getEvents())
	{
		BWAPI::Unit* unit = gameEvent.getUnit();

		switch(gameEvent.getType())
		{
			case BWAPI::EventType::UnitComplete:
				
				if (unit->getPlayer() == BWAPI::Broodwar->self())
				{
					handleOurUnitCompleted(unit);
				}

				break;

			case BWAPI::EventType::UnitMorph:
				if (unit->getPlayer() == BWAPI::Broodwar->self())
				{
					handleOurUnitCompleted(unit);
				}
				break;
			default:

				break;
		}
	}
}


void WorldModel::handleOurUnitCompleted(BWAPI::Unit* unit)
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
	delete this->myArmyVector;
}
