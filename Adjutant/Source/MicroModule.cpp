#include "MicroModule.h"

MicroModule::MicroModule(void)
{
}

void MicroModule::evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue)
{
	std::vector<UnitGroup*>* myArmyGroups = worldModel->myArmyGroups;
	UnitGroup* baseGroup = myArmyGroups->front();

	if (worldModel->isTerrainAnalyzed)
	{
		//Init baseGroup location
		if (baseGroup->targetPosition == BWAPI::Position(0,0))
		{
			baseGroup->targetPosition = worldModel->myHomeRegion->getCenter();
		}

		if (BWAPI::Broodwar->getFrameCount() % 100 == 0)
		{
			BWAPI::Broodwar->printf("MyArmy=%d () | EnemyArmy=%d ()",
				worldModel->myArmyVector->size(),
				worldModel->enemy->getUnits().size());
		}

		//For now, just have the whole army in group(1)
		if (myArmyGroups->size() < 2)
		{
			myArmyGroups->push_back(new UnitGroup());
		}

		if (baseGroup->size() > 0)
		{
			for each (BWAPI::Unit* unit in (*baseGroup->unitVector))
			{
				myArmyGroups->at(1)->addUnit(unit);
				actionQueue->push(new AttackAction(
					unit, 
					myArmyGroups->at(1)->targetPosition
					));
			}

			baseGroup->removeAllUnits();
		}
		
		//Determine army behavior
		BWAPI::Position armyPosition = BWAPI::Position(0,0);

		//No threats
		if (worldModel->enemy->getUnits().size() == 0)
		{
			//TODO:account for multiple chokepoints
			if (worldModel->myHomeRegion->getChokepoints().size() > 0)
			{
				armyPosition = (*worldModel->myHomeRegion->getChokepoints().begin())->getCenter();
			}
			else
			{
				armyPosition = worldModel->myHomeRegion->getCenter();
			}
		}
		else
		{
			//Mob Attack threat location
			armyPosition = (*worldModel->enemy->getUnits().begin())->getPosition();
		}

		myArmyGroups->at(1)->targetPosition = armyPosition;

		//Attack to location for all not near it
		for each (BWAPI::Unit* unit in (*myArmyGroups->at(1)->unitVector))
		{
			if (unit->getDistance(myArmyGroups->at(1)->targetPosition) > 100)
			{
				actionQueue->push(new AttackAction(
					unit, 
					myArmyGroups->at(1)->targetPosition
					));
			}
		}
	}
}

MicroModule::~MicroModule(void)
{
}
