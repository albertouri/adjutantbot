#include "TrainUnitAction.h"

TrainUnitAction::TrainUnitAction(int priority, BWAPI::Unit* building, BWAPI::UnitType* unitType)
{
	this->priority = priority;
	this->building = building;
	this->unitType = unitType;
}

bool TrainUnitAction::isReady()
{
	if (BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed() < unitType->supplyRequired())
	{
		//If there is not enough supply
		return false;
	}
	else if (BWAPI::Broodwar->self()->minerals() < unitType->mineralPrice()
		&& BWAPI::Broodwar->self()->gas() < unitType->gasPrice())
	{
		//If there is not enough resources
		return false;
	}
	else
	{
		return true;
	}
}

bool TrainUnitAction::isStillValid()
{
	//Check to to see if the building still exists
	if (! building->exists())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void TrainUnitAction::execute()
{
	this->building->train(*this->unitType);
}