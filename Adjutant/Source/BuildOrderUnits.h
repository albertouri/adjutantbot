#pragma once
#include <BWAPI.h>

class BuildOrderUnits
{
public:
	BuildOrderUnits(void);
	~BuildOrderUnits(void);
	
	void setUnitRatio(BWAPI::UnitType unitType, int ratio);
	std::map<BWAPI::UnitType, float> getUnitRatioNormalized();
	std::map<BWAPI::UnitType, float> BuildOrderUnits::getWhatBuildsNormalized();
	void clear();

	//List of tech types to be researched for these units
	std::vector<BWAPI::TechType> techTypeVector;

	//List of upgrades to be researched for these units
	std::vector<BWAPI::UpgradeType> upgradeTypeVector;

	//When this limit is reached, transition to the next set of build order
	//units. This is checked against everything (including workers and other units)
	int supplyLimit;

private:
	//List of unit types to produce and their ratio
	std::map<BWAPI::UnitType, int> unitRatioMap;

	//List of unit types to produce and their normalized ratio (all add up to 1)
	std::map<BWAPI::UnitType, float> unitRatioMapNormalized;

	//List of buildings that can train the specified units weighted by ratio and build time
	std::map<BWAPI::UnitType, float> whatBuildsMapNormalized;

	//Indicates if normalized version of the ratios is up to date
	bool isUnitRatioValid;
	bool isWhatBuildsValid;
};
