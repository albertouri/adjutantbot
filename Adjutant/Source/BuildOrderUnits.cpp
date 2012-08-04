#include "BuildOrderUnits.h"

BuildOrderUnits::BuildOrderUnits(void)
{
	this->isUnitRatioValid = false;
	this->isWhatBuildsValid = false;
}

void BuildOrderUnits::setUnitRatio(BWAPI::UnitType unitType, int ratio)
{
	this->unitRatioMap[unitType] = ratio;
	this->isUnitRatioValid = false;
	this->isWhatBuildsValid = false;
}

std::map<BWAPI::UnitType, float> BuildOrderUnits::getUnitRatioNormalized()
{
	//Only recompute if needed
	if (! this->isUnitRatioValid)
	{
		this->unitRatioMapNormalized.clear();
		float totalUnits = 0;

		//Compute total
		for each (std::pair<BWAPI::UnitType, int> pair in this->unitRatioMap)
		{
			totalUnits += pair.second;
		}

		if (totalUnits > 0)
		{
			//Update cached version of map
			for each (std::pair<BWAPI::UnitType, int> pair in this->unitRatioMap)
			{
				this->unitRatioMapNormalized[pair.first] += (float)pair.second / totalUnits;
			}
		}

		this->isUnitRatioValid = true;
	}

	return this->unitRatioMapNormalized;;
}

std::map<BWAPI::UnitType, float> BuildOrderUnits::getWhatBuildsNormalized()
{
	//Only recompute if needed
	if (! this->isWhatBuildsValid)
	{
		this->whatBuildsMapNormalized.clear();
		float totalWeightedBuildTime = 0;

		//Compute total
		for each (std::pair<BWAPI::UnitType, int> pair in this->unitRatioMap)
		{
			totalWeightedBuildTime += (pair.second * pair.first.buildTime());
		}

		if (totalWeightedBuildTime > 0)
		{
			//Update cached version of map
			for each (std::pair<BWAPI::UnitType, int> pair in this->unitRatioMap)
			{
				this->whatBuildsMapNormalized[pair.first.whatBuilds().first] += (float)pair.second / totalWeightedBuildTime;
			}
		}

		this->isWhatBuildsValid = true;
	}

	return this->whatBuildsMapNormalized;
}

void BuildOrderUnits::clear()
{
	this->unitRatioMap.clear();
	this->isUnitRatioValid = false;
	this->isWhatBuildsValid = false;
}

BuildOrderUnits::~BuildOrderUnits(void)
{
}
