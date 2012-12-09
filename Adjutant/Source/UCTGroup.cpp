#include "UCTGroup.h"

UCTGroup::UCTGroup(void)
{
	this->positionX = -1;
	this->positionY = -1;
	this->groupId = -1;
}

UCTGroup::UCTGroup(int groupId, UnitGroup* group)
{
	this->groupId = groupId;

	for each(BWAPI::Unit* unit in *(group->unitVector))
	{
		this->unitTypeMap[unit->getType()]++;
		unitVector.push_back(new HistoricalUnitInfo(
			unit->getID(), 
			unit->getType(), 
			unit->getPosition(),
			unit->getHitPoints())
			);
	}

	BWAPI::Position groupCenter = group->getCentroid();
	this->positionX = groupCenter.x();
	this->positionY = groupCenter.y();
}

UCTGroup::UCTGroup(int groupId, Threat* threat)
{
	this->groupId = groupId;

	for each(BWAPI::Unit* unit in threat->getUnits())
	{
		this->unitTypeMap[unit->getType()]++;
		unitVector.push_back(new HistoricalUnitInfo(
			unit->getID(), 
			unit->getType(), 
			unit->getPosition(),
			unit->getHitPoints())
			);
	}

	BWAPI::Position groupCenter = threat->getCentroid();
	this->positionX = groupCenter.x();
	this->positionY = groupCenter.y();
}

float UCTGroup::getEffectiveHealth()
{
	float ret = 0;
	
	for each (HistoricalUnitInfo* unit in this->unitVector)
	{
		ret += std::sqrt((float)unit->getHitPoints());
	}

	ret = std::pow(ret, 2);

	return ret;
}

float UCTGroup::getEffectiveResourceValue()
{
	float ret = 0;
	
	for each (HistoricalUnitInfo* unit in this->unitVector)
	{
		// (% of HP) * (resource cost)
		ret += ((float)unit->getHitPoints() / (float)unit->getType().maxHitPoints()) 
			* (unit->getType().mineralPrice() + unit->getType().gasPrice());
	}

	return ret;
}

double UCTGroup::getSpeed()
{
	if (this->unitTypeMap.empty()) {return 0;}

	double averageSpeed = 0;
	
	for each (std::pair<BWAPI::UnitType, int> pair in this->unitTypeMap)
	{
		averageSpeed += pair.first.topSpeed();
	}

	averageSpeed /= this->unitTypeMap.size();

	return averageSpeed;
}

double UCTGroup::getDistance(UCTGroup* otherGroup)
{
	return this->getDistanceBetweenPoints(
		this->positionX,
		this->positionY,
		otherGroup->positionX,
		otherGroup->positionY);
}

void UCTGroup::moveTowards(UCTGroup* otherGroup, int frames)
{
	this->moveTowards(otherGroup->positionX, otherGroup->positionY, frames);
}

void UCTGroup::moveTowards(int otherX, int otherY, int frames)
{
	double speed = this->getSpeed();
	double distanceCovered = speed * frames;
	double distanceToCover = this->getDistanceBetweenPoints(
		this->positionX,
		this->positionY,
		otherX,
		otherY);

	//If close enough, move to spot
	//otherwise, interpolate
	if (distanceToCover < distanceCovered)
	{
		this->positionX = otherX;
		this->positionY = otherY;
	}
	else
	{
		double x0 = this->positionX;
		double y0 = this->positionY;
		double x1 = otherX;
		double y1 = otherY;
		
		//Vertical movement only
        if (x0 == x1)
        {
            if (y0 < y1)
			{
				y0 = y0 + distanceCovered;
			}
			else
			{
				y0 = y0 - distanceCovered;
			}
        }
		//Formuala used from UCT.cpp at http://code.google.com/p/tactical-planning-mc-uct/
        //if slope is well-defined, use the standard formula for linear interpolation of distance:
        //  X_t = X_1 + S_t/sqrt(1+m^2)
        //  Y_t = Y_1 + m*S_t/sqrt(1+m^2)
        else
        {
			double slope = (y1 - y0)/(x1 - x0);
            double xDiff = distanceCovered/sqrt(1+(pow(slope, 2)));
			double yDiff = abs((slope*distanceCovered)/sqrt(1+(pow(slope, 2))));
			
			if (x0 < x1)
			{
				x0 = x0 + xDiff;
			}
			else
			{
				x0 = x0 - xDiff;
			}

			if (y0 < y1)
			{
				y0 = y0 + yDiff;
			}
			else
			{
				y0 = y0 - yDiff;
			}
        }

		this->positionX = (int)x0;
		this->positionY = (int)y0;
	}
}

void UCTGroup::merge(UCTGroup* otherGroup)
{
	bool initiallyEmpty = otherGroup->unitVector.size() == 0;

	for each(HistoricalUnitInfo* unit in otherGroup->unitVector)
	{
		this->unitTypeMap[unit->getType()]++;
		unitVector.push_back(new HistoricalUnitInfo(
			unit->getID(), 
			unit->getType(), 
			unit->getPosition(),
			unit->getHitPoints())
			);
	}

	this->positionX += otherGroup->positionX;
	this->positionY += otherGroup->positionY;

	if (! initiallyEmpty)
	{
		this->positionX /= 2;
		this->positionY /= 2;
	}
}

double UCTGroup::getDistanceBetweenPoints(int x0, int y0, int x1, int y1)
{
	double x = x1 - x0;
	double y = y1 - y0;

	return sqrt(pow(x, 2) + pow(y,2));
}

void UCTGroup::removeUnit(HistoricalUnitInfo* unit)
{
	Utils::vectorRemoveElement(&this->unitVector, unit);
	this->unitTypeMap[unit->getType()]--;
	delete unit;
}

UCTGroup::~UCTGroup(void)
{
	for each (HistoricalUnitInfo* hui in this->unitVector)
	{
		delete hui;
	}
}
