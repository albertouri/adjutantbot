#include "UnitGroup.h"

UnitGroup::UnitGroup(void)
{
	this->unitVector = new std::vector<BWAPI::Unit*>();
	this->targetPosition = BWAPI::Positions::None;
}

BWAPI::Position UnitGroup::getCentroid()
{
	if (this->unitVector->size() == 0) {return BWAPI::Positions::None;}

	double centerX = 0.0;
	double centerY = 0.0;

	for each (BWAPI::Unit* unit in (*this->unitVector))
	{
		centerX += unit->getPosition().x();
		centerY += unit->getPosition().y();
	}

	centerX /= this->unitVector->size();
	centerY /= this->unitVector->size();

	return BWAPI::Position((int)centerX, (int)centerY);
}

float UnitGroup::getEffectiveHealth()
{
	float ret = 0;
	
	for each (BWAPI::Unit* unit in (*this->unitVector))
	{
		ret += std::sqrt((float)unit->getHitPoints());
	}

	ret = std::pow(ret, 2);

	return ret;
}

void UnitGroup::addUnit(BWAPI::Unit* unit)
{
	this->unitVector->push_back(unit);
}

bool UnitGroup::removeUnit(BWAPI::Unit* unit)
{
	return Utils::vectorRemoveElement(this->unitVector, unit);
}

void UnitGroup::merge(UnitGroup* otherGroup)
{
	for each (BWAPI::Unit* unit in (*otherGroup->unitVector))
	{
		this->addUnit(unit);
	}
}

int UnitGroup::removeType(BWAPI::UnitType type)
{
	int count = 0;
	std::set<BWAPI::Unit*> unitsToRemove;

	for each (BWAPI::Unit* unit in (*this->unitVector))
	{
		if (unit->getType() == type)
		{
			unitsToRemove.insert(unit);
		}
	}

	for each (BWAPI::Unit* unit in unitsToRemove)
	{
		Utils::vectorRemoveElement(this->unitVector, unit);
		count++;
	}

	return count;
}

void UnitGroup::removeAllUnits()
{
	this->unitVector->clear();
}

int UnitGroup::size()
{
	return this->unitVector->size();
}

UnitGroup::~UnitGroup(void)
{
	delete this->unitVector;
}
