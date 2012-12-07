#pragma once
#include <cmath>
#include "BWAPI.h"
#include "Utils.h"
#include <vector>

class UnitGroup
{
public:
	UnitGroup(void);
	~UnitGroup(void);
	void addUnit(BWAPI::Unit* unit);
	BWAPI::Position getCentroid();
	bool removeUnit(BWAPI::Unit* unit);
	void merge(UnitGroup* otherGroup);
	int removeType(BWAPI::UnitType type);
	void removeAllUnits();
	int size();
	float getEffectiveHealth();

	std::vector<BWAPI::Unit*>* unitVector;
	BWAPI::Position targetPosition;
};
