#include "TrainUnitAction.h"

TrainUnitAction::TrainUnitAction(int priority, BWAPI::Unit* building, BWAPI::UnitType unitType)
{
	this->priority = priority;
	this->building = building;
	this->unitType = unitType;
}

bool TrainUnitAction::isReady(int minerals, int gas, int supplyRemaining)
{

	if (! this->building->isCompleted()
		|| (supplyRemaining < this->unitType.supplyRequired())
		|| (this->unitType.mineralPrice() != 0 && minerals < this->unitType.mineralPrice())
		|| (this->unitType.gasPrice() != 0 && gas < this->unitType.gasPrice())
		)
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
	this->building->train(this->unitType);
}

void TrainUnitAction::updateResourceCost(int* minerals, int* gas, int* supplyRemaining)
{
	*minerals = *minerals - this->unitType.mineralPrice();
	*gas = *gas - this->unitType.gasPrice();
	*supplyRemaining = *supplyRemaining - this->unitType.supplyRequired();
}

bool TrainUnitAction::operator==(const Action &other) const
{
	if (typeid(other) != typeid(TrainUnitAction)) {return false;}
	TrainUnitAction* otherAction = (TrainUnitAction*)&other;

	return (this->building == otherAction->building
		&& this->unitType == otherAction->unitType);
}

std::string TrainUnitAction::toString()
{
	std::string isStillValidText = (this->isStillValid() ? "T" : "F");
	std::string priorityText;

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	return "[P:" + priorityText + "]"
		+ "[V:" + isStillValidText + "]"
		+ " TrainUnitAction"
		+ " " + this->unitType.c_str();
}