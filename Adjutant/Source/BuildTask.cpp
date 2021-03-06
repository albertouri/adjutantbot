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
	this->frameStarted = -1;
}

bool BuildTask::isReady(int minerals, int gas, int supplyRemaining)
{
	BWAPI::UnitType typeNeeded = BWAPI::UnitTypes::None;
	int mineralCost = 0;
	int gasCost = 0;
	int supplyCost = 0;
	bool ret = true;
	

	if (this->isTech())
	{
		mineralCost = this->techType.mineralPrice();
		gasCost = this->techType.gasPrice();
		typeNeeded = this->techType.whatResearches();
	}
	else if (this->isUpgrade())
	{
		mineralCost = this->upgradeType.mineralPrice();
		gasCost = this->upgradeType.gasPrice();
		typeNeeded = this->upgradeType.whatUpgrades();
	}
	else if (this->isConstructBuilding())
	{
		mineralCost = this->unitType.mineralPrice();
		gasCost = this->unitType.gasPrice();
		
		if (! Utils::canMakeGivenUnits(this->unitType))
		{
			ret = false;
		}
	}
	else if (this->isTrainUnit())
	{
		mineralCost = this->unitType.mineralPrice();
		gasCost = this->unitType.gasPrice();
		supplyCost = this->unitType.supplyRequired();
		typeNeeded = this->unitType.whatBuilds().first;
	}

	//Check if required building is ready for training/upgrading/teching
	if (this->buildingToUse != NULL && ! Utils::isBuildingReady(this->buildingToUse))
	{
		ret = false;
	}
	else if (typeNeeded != BWAPI::UnitTypes::None)
	{
		if (WorldManager::Instance().myUnitMap[typeNeeded].empty())
		{
			ret = false;
		}
		else
		{
			bool isOneAvailable = false;

			for each (BWAPI::Unit* building in WorldManager::Instance().myUnitMap[typeNeeded])
			{
				if (Utils::isBuildingReady(building))
				{
					isOneAvailable = true;
					break;
				}
			}

			if (! isOneAvailable)
			{
				ret = false;
			}
		}
	}

	//Resource constraints
	if (std::max(minerals, 0) < mineralCost 
		|| std::max(gas, 0) < gasCost 
		|| std::max(supplyRemaining, 0) < supplyCost)
	{
		ret = false;
	}

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
