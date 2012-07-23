#pragma once
#include "BuildOrderUnits.h"

class BuildOrder
{
public:
	enum GamePhase {Early = 0, Mid = 1 , Late = 2};
	BuildOrder(std::string name);
	~BuildOrder(void);
	BuildOrderUnits* getCurrentUnits();
	void checkForTransition();
	void add(GamePhase phase, BWAPI::UnitType unitType, int ratio); 
	void add(GamePhase phase, BWAPI::TechType techType);
	void add(GamePhase phase, BWAPI::UpgradeType upgradeType);
	void setSupplyLimit(GamePhase phase, int limit);

	std::string name;

private:
	std::vector<BuildOrderUnits*> unitsVector;
	GamePhase currentPhase;
};
