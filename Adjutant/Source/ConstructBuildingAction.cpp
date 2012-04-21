#include "ConstructBuildingAction.h"

//TODO:Update constructor to take in  "building type" and "location"
ConstructBuildingAction::ConstructBuildingAction(int priority, BWAPI::TilePosition loc, BWAPI::UnitType unitType)
{
	this->priority = priority;
	this->location = loc;
	this->buildingType = unitType;
}


ConstructBuildingAction::~ConstructBuildingAction(void)
{
}

bool ConstructBuildingAction::isReady()
{
	if (BWAPI::Broodwar->self()->minerals() < buildingType.mineralPrice()
		&& BWAPI::Broodwar->self()->gas() < buildingType.gasPrice())
	{
		//If there is not enough resources
		return false;
	}
	else
	{
		return true;
	}
}

bool ConstructBuildingAction::isStillValid()
{
	//Check to to see if the location still exists
	if (! this->location.isValid())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ConstructBuildingAction::execute()
{
	// Get Unit to perform work
	BWAPI::Unit* workerPerformingBuild=NULL;

	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		if ((*unit)->getType().isWorker() && !(*unit)->isConstructing())
		{
			workerPerformingBuild = (*unit);
			workerPerformingBuild->build(location, this->buildingType);
			break;
		}
	}
}

std::string ConstructBuildingAction::toString()
{
	std::string isStillValidText = (this->isStillValid() ? "T" : "F");
	std::string isReadyText = (this->isReady() ? "T" : "F");
	std::string priorityText;

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	return "[P:" + priorityText + "]"
		+ "[R:" + isReadyText + "]"
		+ "[V:" + isStillValidText + "]"
		+ " ConstructBuildingAction"
		+ " " + this->buildingType.c_str();
}