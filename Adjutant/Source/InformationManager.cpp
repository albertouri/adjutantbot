#include "InformationManager.h"

InformationManager::InformationManager(void)
{
	this->initUnitCounters();
}

void InformationManager::evaluate()
{
	Utils::log("Entering InformationManager", 1);
	this->manageThreatDetection();
	this->manageUnitCountering();
	Utils::log("Leaving InformationManager", 1);
}

void InformationManager::manageThreatDetection()
{
	const int MAX_CLUSTER_DISTANCE = 300;
	std::set<BWAPI::Unit*> threatUnits;
	std::map<BWAPI::Unit*, Threat*> unitThreatGroupMap;

	//For now, clear previous threats (no memory)
	for each (Threat* t in WorldManager::Instance().threatVector)
	{
		if (t->getUnits().size() == 0)
		{
			delete t;
		}
	}

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

		for each (Threat* t in WorldManager::Instance().threatVector)
		{
			if (Utils::setContains(&(t->getUnits()), unit))
			{
				unitThreatGroupMap[unit] = t;
				break;
			}
		}
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

void  InformationManager::manageUnitCountering()
{
	std::map<BWAPI::UnitType, float> suggestedUnitMap;
	std::map<BWAPI::UnitType, float> enemyTypeMap;
	int totalEnemyType = 0;

	//Get the count each enemy type (that we care about)
	for each (std::pair<int, HistoricalUnitInfo> huiPair in WorldManager::Instance().enemyHistoricalUnitMap)
	{
		BWAPI::UnitType type = huiPair.second.getType();

		if (Utils::mapContains(&this->unitCounters, &type))
		{
			enemyTypeMap[type] += 1;
			totalEnemyType++;
		}
	}

	//Don't know anything about the enemy yet - nothing to do here
	if (totalEnemyType == 0)
	{
		return;
	}

	//Normalize enemy type map
	for each (std::pair<BWAPI::UnitType, float> pair in enemyTypeMap)
	{
		enemyTypeMap[pair.first] /= totalEnemyType;
	}

	//Find counter units for us to use
	for each (std::pair<BWAPI::UnitType, float> pair in enemyTypeMap)
	{
		BWAPI::UnitType enemyType = pair.first;
		float enemyTypeWeight = pair.second;

		for each (std::pair<BWAPI::UnitType, float> counterPair in this->unitCounters[enemyType])
		{
			BWAPI::UnitType counterType = counterPair.first;
			float strength = counterPair.second;

			if (counterType.getRace() == BWAPI::Broodwar->self()->getRace())
			{
				suggestedUnitMap[counterType] += (strength * enemyTypeWeight);
			}
		}
	}

	//Go through suggested counters and see which ones would be counter-countered by current enemy units
	for each (std::pair<BWAPI::UnitType, float> pair in suggestedUnitMap)
	{
		BWAPI::UnitType suggestedType = pair.first;

		for each (std::pair<BWAPI::UnitType, float> counterPair in this->unitCounters[suggestedType])
		{
			BWAPI::UnitType counterType = counterPair.first;
			float strength = counterPair.second;

			if (counterType.getRace() == BWAPI::Broodwar->self()->getRace())
			{
				if (counterType != BWAPI::UnitTypes::Terran_Marine)
				{
					suggestedUnitMap[counterType] -= strength * enemyTypeMap[counterType];
				}
			}
		}
	}

	float totalSuggestedUnits = 0;

	for each (std::pair<BWAPI::UnitType, float> pair in suggestedUnitMap)
	{
		if (pair.second > 0)
		{
			totalSuggestedUnits += pair.second;
		}
	}

	if (WorldManager::Instance().buildOrder != NULL && totalSuggestedUnits != 0)
	{
		BuildOrderUnits* orderUnits = WorldManager::Instance().buildOrder->getCurrentUnits();

		//Completely reactive at this point (replaces entire build order)
		orderUnits->clear();

		for each (std::pair<BWAPI::UnitType, float> pair in suggestedUnitMap)
		{		
			if (pair.second > 0)
			{
				orderUnits->setUnitRatio(pair.first, (int)((pair.second / (float)totalSuggestedUnits) * 100.0f));
			}
		}
	}
}

InformationManager::~InformationManager(void)
{
}

void InformationManager::initUnitCounters()
{
	//START - this code generated by excel macro
	this->unitCounters[BWAPI::UnitTypes::Protoss_Arbiter][BWAPI::UnitTypes::Terran_Goliath] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Arbiter][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Arbiter][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Archon][BWAPI::UnitTypes::Terran_Science_Vessel] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Archon][BWAPI::UnitTypes::Terran_Vulture] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Carrier][BWAPI::UnitTypes::Terran_Battlecruiser] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Carrier][BWAPI::UnitTypes::Terran_Goliath] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Carrier][BWAPI::UnitTypes::Terran_Wraith] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Corsair][BWAPI::UnitTypes::Terran_Goliath] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Corsair][BWAPI::UnitTypes::Terran_Marine] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Corsair][BWAPI::UnitTypes::Terran_Valkyrie] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dark_Archon][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dark_Archon][BWAPI::UnitTypes::Terran_Vulture] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dark_Templar][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dark_Templar][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dragoon][BWAPI::UnitTypes::Terran_Marine] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dragoon][BWAPI::UnitTypes::Terran_Medic] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dragoon][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Dragoon][BWAPI::UnitTypes::Terran_Vulture] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_High_Templar][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_High_Templar][BWAPI::UnitTypes::Terran_Vulture] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Observer][BWAPI::UnitTypes::Terran_Missile_Turret] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Observer][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Photon_Cannon][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Reaver][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Scout][BWAPI::UnitTypes::Terran_Goliath] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Scout][BWAPI::UnitTypes::Terran_Marine] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Scout][BWAPI::UnitTypes::Terran_Medic] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Scout][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Scout][BWAPI::UnitTypes::Terran_Valkyrie] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Shuttle][BWAPI::UnitTypes::Terran_Missile_Turret] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Shuttle][BWAPI::UnitTypes::Terran_Wraith] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Zealot][BWAPI::UnitTypes::Terran_Firebat] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Zealot][BWAPI::UnitTypes::Terran_Medic] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Protoss_Zealot][BWAPI::UnitTypes::Terran_Vulture] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Protoss_Arbiter] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Protoss_Dark_Archon] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Protoss_Dragoon] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Protoss_Photon_Cannon] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Protoss_Scout] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Terran_Goliath] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Terran_Marine] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Terran_Medic] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Terran_Wraith] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Zerg_Defiler] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Zerg_Devourer] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Zerg_Hydralisk] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Zerg_Mutalisk] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Battlecruiser][BWAPI::UnitTypes::Zerg_Scourge] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Bunker][BWAPI::UnitTypes::Protoss_Corsair] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Bunker][BWAPI::UnitTypes::Protoss_Reaver] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Bunker][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Bunker][BWAPI::UnitTypes::Zerg_Guardian] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Protoss_Corsair] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Protoss_Photon_Cannon] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Protoss_Scout] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Terran_Goliath] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Terran_Wraith] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Zerg_Mutalisk] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Zerg_Scourge] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Dropship][BWAPI::UnitTypes::Zerg_Spore_Colony] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Protoss_Dragoon] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Protoss_High_Templar] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Terran_Goliath] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Terran_Vulture] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Zerg_Hydralisk] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Zerg_Lurker] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Firebat][BWAPI::UnitTypes::Zerg_Ultralisk] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Ghost][BWAPI::UnitTypes::Protoss_Observer] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Ghost][BWAPI::UnitTypes::Protoss_Photon_Cannon] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Ghost][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Ghost][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Ghost][BWAPI::UnitTypes::Zerg_Spore_Colony] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Protoss_Dragoon] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Protoss_Reaver] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Protoss_Zealot] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Terran_Marine] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Terran_Medic] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Zerg_Hydralisk] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Zerg_Mutalisk] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Goliath][BWAPI::UnitTypes::Zerg_Zergling] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Protoss_Dragoon] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Protoss_High_Templar] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Protoss_Zealot] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Terran_Marine] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Terran_Medic] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Zerg_Hydralisk] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Zerg_Lurker] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Marine][BWAPI::UnitTypes::Zerg_Zergling] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Missile_Turret][BWAPI::UnitTypes::Zerg_Guardian] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Science_Vessel][BWAPI::UnitTypes::Protoss_Dark_Archon] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Science_Vessel][BWAPI::UnitTypes::Terran_Goliath] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Science_Vessel][BWAPI::UnitTypes::Terran_Marine] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Science_Vessel][BWAPI::UnitTypes::Terran_Medic] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Science_Vessel][BWAPI::UnitTypes::Zerg_Scourge] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Protoss_Dark_Templar] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Protoss_Reaver] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Protoss_Zealot] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Terran_Marine] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Terran_Medic] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Terran_Wraith] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Zerg_Queen] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode][BWAPI::UnitTypes::Zerg_Zergling] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Valkyrie][BWAPI::UnitTypes::Protoss_Scout] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Valkyrie][BWAPI::UnitTypes::Terran_Battlecruiser] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Valkyrie][BWAPI::UnitTypes::Zerg_Devourer] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Valkyrie][BWAPI::UnitTypes::Zerg_Scourge] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Vulture][BWAPI::UnitTypes::Protoss_Dragoon] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Vulture][BWAPI::UnitTypes::Terran_Goliath] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Vulture][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Vulture][BWAPI::UnitTypes::Zerg_Hydralisk] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Vulture][BWAPI::UnitTypes::Zerg_Lurker] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Vulture][BWAPI::UnitTypes::Zerg_Ultralisk] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Protoss_Dragoon] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Protoss_Photon_Cannon] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Protoss_Scout] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Terran_Goliath] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Terran_Marine] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Terran_Medic] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Terran_Valkyrie] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Zerg_Devourer] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Terran_Wraith][BWAPI::UnitTypes::Zerg_Spore_Colony] = 0.3f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Defiler][BWAPI::UnitTypes::Terran_Firebat] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Defiler][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Defiler][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Guardian][BWAPI::UnitTypes::Terran_Goliath] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Guardian][BWAPI::UnitTypes::Terran_Valkyrie] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Guardian][BWAPI::UnitTypes::Terran_Wraith] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Hydralisk][BWAPI::UnitTypes::Terran_Marine] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Hydralisk][BWAPI::UnitTypes::Terran_Medic] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Hydralisk][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.8f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Infested_Terran][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Lurker][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Lurker][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Mutalisk][BWAPI::UnitTypes::Terran_Marine] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Mutalisk][BWAPI::UnitTypes::Terran_Medic] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Mutalisk][BWAPI::UnitTypes::Terran_Valkyrie] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Queen][BWAPI::UnitTypes::Terran_Missile_Turret] = 0.5f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Queen][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Scourge][BWAPI::UnitTypes::Terran_Marine] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Scourge][BWAPI::UnitTypes::Terran_Medic] = 0.4f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Sunken_Colony][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Ultralisk][BWAPI::UnitTypes::Terran_Marine] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Ultralisk][BWAPI::UnitTypes::Terran_Medic] = 0.6f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Ultralisk][BWAPI::UnitTypes::Terran_Science_Vessel] = 0.2f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Ultralisk][BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode] = 0.9f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Zergling][BWAPI::UnitTypes::Terran_Firebat] = 1.0f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Zergling][BWAPI::UnitTypes::Terran_Marine] = 0.7f;
	this->unitCounters[BWAPI::UnitTypes::Zerg_Zergling][BWAPI::UnitTypes::Terran_Medic] = 1.0f;
	//STOP - this code generated by excel macro
}