#include "InformationManager.h"

InformationManager::InformationManager(void)
{
}

void InformationManager::evaluate()
{
	this->manageThreatDetection();
}

void InformationManager::manageThreatDetection()
{
	const int MAX_CLUSTER_DISTANCE = 300;
	std::set<BWAPI::Unit*> threatUnits;
	std::map<BWAPI::Unit*, Threat*> unitThreatGroupMap;

	//For now, clear previous threats (no memory)
	for each (Threat* t in WorldManager::Instance().threatVector)
	{
		delete t;
	}
	WorldManager::Instance().threatVector.clear();

	//Determine threat units
	for each (BWAPI::Unit* unit in WorldManager::Instance().enemy->getUnits())
	{
		bool inProtectedRegion = Utils::vectorContains(
			&WorldManager::Instance().protectedRegionVector, 
			BWTA::getRegion(unit->getPosition()));

		if (inProtectedRegion || ! unit->getType().isBuilding())
		{
			threatUnits.insert(unit);
		}
	}

	//TODO: Could make this much more effecient
	//Determine threat groups using simple agglomerative hiearchial clustering
	for each (BWAPI::Unit* unit in threatUnits)
	{
		unitThreatGroupMap[unit] = NULL;
	}

	for each (BWAPI::Unit* unit in threatUnits)
	{
		if (unitThreatGroupMap[unit] != NULL) {continue;}

		for each (BWAPI::Unit* otherUnit in threatUnits)
		{
			if (unit == otherUnit) {continue;}

			if (unit->getDistance(otherUnit) < MAX_CLUSTER_DISTANCE)
			{
				if (unitThreatGroupMap[otherUnit] != NULL)
				{
					unitThreatGroupMap[unit] = unitThreatGroupMap[otherUnit];
					unitThreatGroupMap[otherUnit]->addUnit(unit);
				}
				else
				{
					Threat* threat = new Threat(BWAPI::Broodwar->getFrameCount());
					threat->addUnit(unit);
					threat->addUnit(otherUnit);

					unitThreatGroupMap[unit] = threat;
					unitThreatGroupMap[otherUnit] = threat;
					WorldManager::Instance().threatVector.push_back(threat);
				}

				break;
			}
		}

		if (unitThreatGroupMap[unit] == NULL)
		{
			Threat* threat = new Threat(BWAPI::Broodwar->getFrameCount());
			threat->addUnit(unit);
			WorldManager::Instance().threatVector.push_back(threat);
		}
	}
}

InformationManager::~InformationManager(void)
{
}