#pragma once
#include "BWAPI.h"
#include "Utils.h"

class BuildTask
{
public:
	BuildTask(void);
	BuildTask::BuildTask(int p, BWAPI::UpgradeType ut);
	BuildTask::BuildTask(int p, BWAPI::TechType tt);
	BuildTask::BuildTask(int p, BWAPI::UnitType unt);
	BuildTask::BuildTask(int p, BWAPI::UnitType unt, BWAPI::TilePosition tp);
	BuildTask::BuildTask(int p, BWAPI::UnitType unt, BWAPI::Unit* buildingToUse);
	~BuildTask(void);

	bool isUpgrade();
	bool isTech();
	bool isTrainUnit();
	bool isConstructBuilding();
	bool BuildTask::isReady(int minerals, int gas, int supplyRemaining);
	void BuildTask::updateResourceCost(int* minerals, int* gas, int* supplyRemaining);
	std::string toString();

	std::string taskTypeText;

	//Only one of these three will be set
	BWAPI::UpgradeType upgradeType;
	BWAPI::TechType techType;
	BWAPI::UnitType unitType;

	//required - priority of this task. "Normal" is 500. Lower is higher priority.
	//-1 is used to fix supply block, so make sure all tasks are >= 0. 
	int priority;

	//optional
	BWAPI::TilePosition position;
	BWAPI::Unit* buildingToUse;
	long minFrame;

private:
	void init(int priority, std::string text, BWAPI::UpgradeType upgradeType, 
		BWAPI::TechType techType, BWAPI::UnitType unitType);
};
