#include "ConstructBuildingAction.h"
#include "WorldManager.h"

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

bool ConstructBuildingAction::isReady(int minerals, int gas, int supplyRemaining)
{

	//Draw Building placement 
	Utils::isValidBuildingLocation(this->location, this->buildingType);

	if (this->buildingType.mineralPrice() != 0 && minerals < this->buildingType.mineralPrice()
		|| (this->buildingType.gasPrice() != 0 && gas < this->buildingType.gasPrice())
		)
	{
		//If there is not enough resources
		return false;
	}
	else if (NULL == Utils::getFreeWorker(WorldManager::Instance().myWorkerVector))
	{
		return false;
	}
	else if (! Utils::isValidBuildingLocation(this->location, this->buildingType))
	{
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
	if (! Utils::isValidBuildingLocation(this->location, this->buildingType))
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
	BWAPI::Unit* workerPerformingBuild = Utils::getFreeWorker(WorldManager::Instance().myWorkerVector);

	if (workerPerformingBuild != NULL)
	{
		//Reserve resources for SCV's travel to building location
		WorldManager::Instance().reservedGas += this->buildingType.gasPrice();
		WorldManager::Instance().reservedMinerals += this->buildingType.mineralPrice();
		WorldManager::Instance().workersBuildingMap[workerPerformingBuild] = new ConstructBuildingAction(this->priority, 
			this->location, 
			this->buildingType);

		workerPerformingBuild->build(location, this->buildingType);

		BWAPI::Broodwar->printf("Worker at %d,%d sent to construct %s at %d,%d",
			workerPerformingBuild->getPosition().x(),
			workerPerformingBuild->getPosition().y(),
			this->buildingType.getName().c_str(),
			this->location.x(),
			this->location.y());
	}
	else
	{
		BWAPI::Broodwar->printf("Executed build but failed to find free worker");
	}
}

void ConstructBuildingAction::updateResourceCost(int* minerals, int* gas, int* supplyRemaining)
{
	*minerals = *minerals - this->buildingType.mineralPrice();
	*gas = *gas - this->buildingType.gasPrice();
	*supplyRemaining = *supplyRemaining - this->buildingType.supplyRequired();
}

bool ConstructBuildingAction::operator==(const Action &other) const
{
	if (typeid(other) != typeid(ConstructBuildingAction)) {return false;}
	ConstructBuildingAction* otherAction = (ConstructBuildingAction*)&other;
	bool isSame = true;

	if (this->buildingType != otherAction->buildingType)
	{
		isSame = false;
	}
	else if (this->location != otherAction->location)
	{
		isSame = false;
	}

	return isSame;
}

std::string ConstructBuildingAction::toString()
{
	std::string isStillValidText = (this->isStillValid() ? "T" : "F");
	std::string priorityText, toX, toY;

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	stream.str("");
	stream << this->location.x();
	toX = stream.str();

	stream.str("");
	stream << this->location.y();
	toY = stream.str();


	return "[P:" + priorityText + "]"
		+ "[V:" + isStillValidText + "]"
		+ " ConstructBuildingAction"
		+ " " + this->buildingType.c_str() + "(" + toX + "," + toY + ")";
}