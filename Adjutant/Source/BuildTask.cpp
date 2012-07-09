#include "BuildTask.h"
#include "WorldManager.h"

BuildTask::BuildTask(int p, BWAPI::UpgradeType upgradeType)
{
	init(p, "Upgrade", upgradeType, BWAPI::TechTypes::None, BWAPI::UnitTypes::None);
}

BuildTask::BuildTask(int p, BWAPI::TechType techType)
{
	init(p, "Tech", BWAPI::UpgradeTypes::None, techType, BWAPI::UnitTypes::None);
}

BuildTask::BuildTask(int p, BWAPI::UnitType unitType)
{
	init(p, "UnitRandom", BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None, unitType);
}

BuildTask::BuildTask(int p, BWAPI::UnitType unitType, BWAPI::TilePosition position)
{
	init(p, "ConstructAtLocation", BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None, unitType);
	this->position = position;
}

BuildTask::BuildTask(int p, BWAPI::UnitType unitType, BWAPI::Unit* buildingToUse)
{
	init(p, "TrainAtBuilding", BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None, unitType);
	this->buildingToUse = buildingToUse;
}

void BuildTask::init(int p, std::string text, BWAPI::UpgradeType upgradeType, BWAPI::TechType techType, BWAPI::UnitType unitType)
{
	this->priority = p;
	this->taskTypeText = text;
	this->upgradeType = upgradeType;
	this->techType = techType;
	this->unitType = unitType;
	this->position = BWAPI::TilePositions::Invalid;
	this->buildingToUse = NULL;
	
}

bool BuildTask::isReady(int minerals, int gas, int supplyRemaining)
{
	bool ret = true;
	int mineralCost = 0;
	int gasCost = 0;
	int supplyCost = 0;

	if (this->isTech())
	{
		mineralCost = this->techType.mineralPrice();
		gasCost = this->techType.gasPrice();
	}
	else if (this->isUpgrade())
	{
		mineralCost = this->upgradeType.mineralPrice();
		gasCost = this->upgradeType.gasPrice();
	}
	else if (this->isConstructBuilding() )
	{
		mineralCost = this->unitType.mineralPrice();
		gasCost = this->unitType.gasPrice();

		//Check to make sure we have a free worker if we are trying to build something
		if (NULL == Utils::getFreeWorker(WorldManager::Instance().myWorkerVector))
		{
			ret = false;
		}
	}
	else if (this->isTrainUnit())
	{
		mineralCost = this->unitType.mineralPrice();
		gasCost = this->unitType.gasPrice();
		supplyCost = this->unitType.supplyRequired();

		if ((this->buildingToUse != NULL && ! this->buildingToUse->isCompleted()))
		{
			ret = false;
		}
	}

	//Global conditions
	if (minerals < mineralCost || gas < gasCost || supplyRemaining < supplyCost)
	{
		ret = false;
	}

	//TODO: Finish checking that all dependencies are built (i.e. factory needs rax)

	return ret;
}

bool BuildTask::isUpgrade()
{
	return (this->upgradeType != BWAPI::UpgradeTypes::None);
}

bool BuildTask::isTech()
{
	return (this->techType != BWAPI::TechTypes::None);
}

bool BuildTask::isTrainUnit()
{
	return (this->unitType != BWAPI::UnitTypes::None && !this->unitType.isBuilding());
}

bool BuildTask::isConstructBuilding()
{
	return (this->unitType != BWAPI::UnitTypes::None && this->unitType.isBuilding());
}

void BuildTask::updateResourceCost(int* minerals, int* gas, int* supplyRemaining)
{
	if (this->unitType != BWAPI::UnitTypes::None)
	{
		*minerals = *minerals - this->unitType.mineralPrice();
		*gas = *gas - this->unitType.gasPrice();
		*supplyRemaining = *supplyRemaining - this->unitType.supplyRequired();
	}
}

std::string BuildTask::toString()
{
	std::string priorityText, toX, toY, ret;

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	ret = "[P:" + priorityText + "]"
		+ " " + this->taskTypeText;

	if (this->unitType != BWAPI::UnitTypes::None)
	{
		ret += " ";
		ret += this->unitType.c_str();
	}
	else if (this->techType != BWAPI::TechTypes::None)
	{
		ret += " ";
		ret += this->techType.c_str();
	}
	else if (this->upgradeType != BWAPI::UpgradeTypes::None)
	{
		ret += " ";
		ret += this->upgradeType.c_str();
	}

	if (this->position != BWAPI::TilePositions::Invalid)
	{
		stream.str("");
		stream << this->position.x();
		toX = stream.str();

		stream.str("");
		stream << this->position.y();
		toY = stream.str();

		ret += "(" + toX + "," + toY + ")";
	}

	return ret;
}

BuildTask::~BuildTask(void)
{
}
