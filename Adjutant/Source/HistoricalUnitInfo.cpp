#include "HistoricalUnitInfo.h"

HistoricalUnitInfo::HistoricalUnitInfo()
{
	this->id = -1;
	this->setPosition(BWAPI::Position(0,0));
	this->setType(BWAPI::UnitTypes::Unknown);
	this->hitPoints = -1;
}

HistoricalUnitInfo::HistoricalUnitInfo(int id, BWAPI::UnitType unitType, BWAPI::Position position)
{
	this->id = id;
	this->setPosition(position);
	this->setType(unitType);
	this->hitPoints = -1;
}

HistoricalUnitInfo::HistoricalUnitInfo(int id, BWAPI::UnitType unitType, BWAPI::Position position, int hitPoints)
{
	this->id = id;
	this->setPosition(position);
	this->setType(unitType);
	this->hitPoints = hitPoints;
}

int HistoricalUnitInfo::getID()
{
	return this->id;
}

void HistoricalUnitInfo::setID(int id)
{
	this->id = id;
}

int HistoricalUnitInfo::getHitPoints()
{
	return this->hitPoints;
}

void HistoricalUnitInfo::setHitPoints(int hitPoints)
{
	this->hitPoints = hitPoints;
}

BWAPI::UnitType HistoricalUnitInfo::getType()
{
	return this->unitType;
}

void HistoricalUnitInfo::setType(BWAPI::UnitType unitType)
{
	this->unitType = unitType;
}

BWAPI::Position HistoricalUnitInfo::getPosition()
{
	return this->position;
}

void HistoricalUnitInfo::setPosition(BWAPI::Position position)
{
	this->position = position;
}


bool HistoricalUnitInfo::operator==(const HistoricalUnitInfo &other) const
{
	if (typeid(other) != typeid(HistoricalUnitInfo)) {return false;}
	HistoricalUnitInfo* otherAction = (HistoricalUnitInfo*)&other;
	bool isSame = true;

	if (otherAction->id != this->id)
	{
		isSame = false;
	}

	return isSame;
}

bool HistoricalUnitInfo::operator!=(const HistoricalUnitInfo &other) const {
	return !(*this == other);
}


HistoricalUnitInfo::~HistoricalUnitInfo(void)
{
}
