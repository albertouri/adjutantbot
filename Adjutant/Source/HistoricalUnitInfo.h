#pragma once
#include "BWAPI.h"

class HistoricalUnitInfo
{
public:
	HistoricalUnitInfo();
	HistoricalUnitInfo(int id, BWAPI::UnitType unitType, BWAPI::Position position);
	HistoricalUnitInfo(int id, BWAPI::UnitType unitType, BWAPI::Position position, int hitPoints);
	~HistoricalUnitInfo(void);

	bool operator==(const HistoricalUnitInfo &other) const;
	bool operator!=(const HistoricalUnitInfo &other) const;

	int getID();
	void setID(int id);
	int getHitPoints();
	void setHitPoints(int hitPoints);
	BWAPI::UnitType getType();
	void setType(BWAPI::UnitType unitType);
	BWAPI::Position getPosition();
	void setPosition(BWAPI::Position position);
protected:
	int id;
	int hitPoints;
	BWAPI::UnitType unitType;
	BWAPI::Position position;
};
