#include "BuildTask.h"
#include "WorldManager.h"

BuildTask::BuildTask(int p, BWAPI::UpgradeType ut)
{
	init(p, "Upgrade", ut, NULL, NULL);
}

BuildTask::BuildTask(int p, BWAPI::TechType tt)
{
	init(p, "Tech", NULL, tt, NULL);
}

BuildTask::BuildTask(int p, BWAPI::UnitType unt)
{
	init(p, "UnitRandom", NULL, NULL, unt);
}

BuildTask::BuildTask(int p, BWAPI::UnitType unt, BWAPI::TilePosition tp)
{
	init(p, "ConstructAtLocation", NULL, NULL, unt);
	this->position = tp;
}

BuildTask::BuildTask(int p, BWAPI::UnitType unt, BWAPI::Unit* btu)
{
	init(p, "TrainAtBuilding", NULL, NULL, unt);
	this->buildingToUse = btu;
}

void BuildTask::init(int p, std::string text, BWAPI::UpgradeType upt, BWAPI::TechType tt, BWAPI::UnitType unt)
{
	this->priority = p;
	this->taskTypeText = text;
	this->upgradeType = upt;
	this->techType = tt;
	this->unitType = unt;
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
	else if (this->isConstructBuilding() || this->isTrainUnit())
	{
		mineralCost = this->unitType.mineralPrice();
		gasCost = this->unitType.gasPrice();
		supplyCost = this->unitType.supplyRequired();

		//Check to make sure we have a free worker if we are trying to build something
		if (this->isConstructBuilding())
		{
			if (NULL == Utils::getFreeWorker(WorldManager::Instance().myWorkerVector))
			{
				ret = false;
			}
		}
	}

	//Global conditions
	if (minerals < mineralCost || gas < gasCost || supplyRemaining < supplyCost)
	{
		ret = false;
	}


	if ((this->buildingToUse != NULL && ! this->buildingToUse->isCompleted()))
	{
		ret = false;
	}

	//TODO: Finish checking that all dependencies are built (i.e. factory needs rax)

	return ret;
}

bool BuildTask::isUpgrade()
{
	return (this->upgradeType != NULL);
}

bool BuildTask::isTech()
{
	return (this->techType != NULL);
}

bool BuildTask::isTrainUnit()
{
	return (this->unitType != NULL && !this->unitType.isBuilding());
}

bool BuildTask::isConstructBuilding()
{
	return (this->unitType != NULL && this->unitType.isBuilding());
}

void BuildTask::updateResourceCost(int* minerals, int* gas, int* supplyRemaining)
{
	if (this->unitType != NULL)
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

	if (this->unitType != NULL)
	{
		ret += " ";
		ret += this->unitType.c_str();
	}
	else if (this->techType != NULL)
	{
		ret += " ";
		ret += this->techType.c_str();
	}
	else if (this->upgradeType != NULL)
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
