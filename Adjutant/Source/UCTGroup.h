#pragma once
#include "BWAPI.h"
#include "HistoricalUnitInfo.h"
#include <math.h>
#include "Threat.h"
#include "UCTAction.h"
#include "UnitGroup.h"
#include "Utils.h"

class UCTGroup
{
public:
	UCTGroup(void);
	UCTGroup(int groupId, UnitGroup* group);
	UCTGroup(int groupId, Threat* group);
	~UCTGroup(void);
	float getEffectiveHealth();
	float getEffectiveResourceValue();
	double getSpeed();
	double getDistance(UCTGroup* otherGroup);
	void moveTowards(UCTGroup* otherGroup, int frames);
	void moveTowards(int x, int y, int frames);
	void merge(UCTGroup* otherGroup);
	void removeUnit(HistoricalUnitInfo* unit);

	std::map<BWAPI::UnitType, int> unitTypeMap;
	std::vector<HistoricalUnitInfo*> unitVector;
	int positionX;
	int positionY;
	int groupId;
private:
	double getDistanceBetweenPoints(int x0, int y0, int x1, int y1);
};
