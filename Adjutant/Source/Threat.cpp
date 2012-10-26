#include "Threat.h"
#include "WorldManager.h"

Threat::Threat(long frame)
{
	this->isVisibleVar = true;
	this->hasAirUnitsVar = false;
	this->hasGroundUnitsVar = false;
	this->hasHidableUnitsVar = false;

	this->lastFrameVisible = frame;
}

void Threat::addUnit(BWAPI::Unit* unit)
{
	this->unitSet.insert(unit);

	BWAPI::UnitType type = unit->getType();

	if (type.isFlyer())
	{
		this->hasAirUnitsVar = true;
	}

	if (! type.isFlyer())
	{
		this->hasGroundUnitsVar = true;
	}

	if (type.hasPermanentCloak() || type.cloakingTech() != BWAPI::TechTypes::None)
	{
		hasHidableUnitsVar = true;
	}
}

BWAPI::Position Threat::getCentroid()
{
	double centerX = 0.0;
	double centerY = 0.0;

	for each (BWAPI::Unit* unit in this->unitSet)
	{
		centerX += unit->getPosition().x();
		centerY += unit->getPosition().y();
	}

	centerX /= this->unitSet.size();
	centerY /= this->unitSet.size();

	return BWAPI::Position((int)centerX, (int)centerY);
}

bool Threat::isInProtectedRegion()
{
	for each (BWAPI::Unit* unit in this->unitSet)
	{
		if (Utils::vectorContains(
			&WorldManager::Instance().protectedRegionVector, 
			BWTA::getRegion(unit->getPosition())))
		{
			return true;
		}
	}

	return false;
}

int Threat::getArmyValue()
{
	int value = 0;

	for each (BWAPI::Unit* unit in this->unitSet)
	{
		value += (unit->getType().gasPrice() + unit->getType().mineralPrice());
	}

	return value;
}

int Threat::getAttackValue()
{
	int value = 0;

	for each (BWAPI::Unit* unit in this->unitSet)
	{
		if (unit->getType().isWorker())
		{
			value += 13;
		}
		else
		{
			value += unit->getType().gasPrice() + unit->getType().mineralPrice();
		}
	}

	return value;
}

int Threat::removeNonExistentUnits()
{
	int removedCount = 0;
	std::set<BWAPI::Unit*> unitToRemoveSet = std::set<BWAPI::Unit*>();

	for each (BWAPI::Unit* unit in this->unitSet)
	{
		if (! unit->exists())
		{
			unitToRemoveSet.insert(unit);
		}
	}

	for each (BWAPI::Unit* unit in unitToRemoveSet)
	{
		Utils::setRemoveElement(&this->unitSet, unit);
		removedCount++;
	}

	return removedCount;
}

Threat::~Threat(void)
{
}
