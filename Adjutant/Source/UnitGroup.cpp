#include "UnitGroup.h"

UnitGroup::UnitGroup(void)
{
	this->unitVector = new std::vector<BWAPI::Unit*>();
	this->targetPosition = BWAPI::Position(0,0);
}

BWAPI::Position UnitGroup::getCentroid()
{
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

void UnitGroup::addUnit(BWAPI::Unit* unit)
{
	this->unitVector->push_back(unit);
}

bool UnitGroup::removeUnit(BWAPI::Unit* unit)
{
	return Utils::vectorRemoveElement(this->unitVector, unit);
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
