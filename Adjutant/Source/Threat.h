#pragma once
#include <BWAPI.h>
#include "Utils.h"


class Threat
{
public:
	Threat(long frame);
	~Threat(void);

	void addUnit(BWAPI::Unit* unit);
	std::set<BWAPI::Unit*> getUnits() {return unitSet;}

	long lastFrameVisible;
	bool isVisible() {return isVisibleVar;}
	bool hasAirUnits() {return hasAirUnitsVar;}
	bool hasGroundUnits() {return hasGroundUnitsVar;}
	bool hasHidableUnits() {return hasHidableUnitsVar;}
	bool isInProtectedRegion();
	BWAPI::Position getCentroid();
	int getAttackValue();
	int getArmyValue();
	int removeNonExistentUnits();

private:
	bool isVisibleVar;
	bool hasAirUnitsVar;
	bool hasGroundUnitsVar;
	bool hasHidableUnitsVar;
	std::set<BWAPI::Unit*> unitSet;
};
