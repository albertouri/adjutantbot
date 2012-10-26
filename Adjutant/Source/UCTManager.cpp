#include "UCTManager.h"

UCTManager::UCTManager(void)
{
	this->roundInProgress = false;
}

void UCTManager::evaluate()
{
	int myUnitCount = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int enemyUnitCount = 0;

	for each (BWAPI::Player* enemy in BWAPI::Broodwar->enemies())
	{
		enemyUnitCount += enemy->completedUnitCount(BWAPI::UnitTypes::Terran_Marine);
	}

	//Check for round beginning and end
	if (this->roundInProgress)
	{
		//Round just ended
		if (myUnitCount == 0 || enemyUnitCount == 0)
		{
			this->onRoundEnd();
			this->roundInProgress = false;
		}
	}
	else
	{
		//Round just started
		if (myUnitCount > 0 && enemyUnitCount > 0)
		{
			this->onRoundStart();
			this->roundInProgress = true;
		}
	}
}

void UCTManager::onRoundStart()
{
	BWAPI::Broodwar->printf("Entering - OnRoundStart()");
	//Form groups
	const int MAX_CLUSTER_DISTANCE = 100;
	std::map<BWAPI::Unit*, UnitGroup*> unitGroupMap;
	std::set<BWAPI::Unit*> myMen = std::set<BWAPI::Unit*>();
	std::vector<UnitGroup*>* unitGroupVector = new std::vector<UnitGroup*>();

	for each (BWAPI::Unit* unit in BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().canMove())
		{
			myMen.insert(unit);
		}
	}

	//TODO: Could be much more effecient
	//Determine threat groups using simple agglomerative hiearchial clustering
	for each (BWAPI::Unit* unit in myMen)
	{
		unitGroupMap[unit] = NULL;
	}

	for each (BWAPI::Unit* unit in myMen)
	{
		if (unitGroupMap[unit] != NULL) {continue;}

		for each (BWAPI::Unit* otherUnit in myMen)
		{
			if (unit == otherUnit) {continue;}

			if (unit->getDistance(otherUnit) < MAX_CLUSTER_DISTANCE)
			{
				if (unitGroupMap[otherUnit] != NULL)
				{
					unitGroupMap[unit] = unitGroupMap[otherUnit];
					unitGroupMap[otherUnit]->addUnit(unit);
				}
				else
				{
					UnitGroup* group = new UnitGroup();
					group->addUnit(unit);
					group->addUnit(otherUnit);

					unitGroupMap[unit] = group;
					unitGroupMap[otherUnit] = group;
					unitGroupVector->push_back(group);
				}

				break;
			}
		}

		if (unitGroupMap[unit] == NULL)
		{
			UnitGroup* group = new UnitGroup();
			group->addUnit(unit);
			unitGroupVector->push_back(group);
		}
	}

	WorldManager::Instance().myArmyGroups = unitGroupVector;
	WorldManager::Instance().groupAttackMap.clear();

	for each(UnitGroup* group in *unitGroupVector)
	{
		int choice = rand() % WorldManager::Instance().threatVector.size(); //randomly choose a threat
		WorldManager::Instance().groupAttackMap[group] = WorldManager::Instance().threatVector.at(choice);
	}
}

void UCTManager::onRoundEnd()
{

}

UCTManager::~UCTManager(void)
{
}
