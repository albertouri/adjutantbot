#include "BuildingManager.h"


BuildingManager::BuildingManager(void)
{
}

void BuildingManager::evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue)
{
	//BWAPI::Broodwar->sendText("Successfully Entered BuildingManager");
	BWAPI::Unit* cc = NULL;
	
	int gasWorkers = 0;
	int mineralWorkers = 0;

	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		// If is Command Center
		if ((*unit)->getType().isResourceDepot())
		{
			// Find closest Geyser
			BWAPI::Unit* closestGeyser=NULL;
			for(std::set<BWAPI::Unit*>::iterator g=BWAPI::Broodwar->getGeysers().begin();g!=BWAPI::Broodwar->getGeysers().end();g++)
			{
				if (closestGeyser==NULL || (*unit)->getDistance(*g) < (*unit)->getDistance(closestGeyser))
				{
					closestGeyser=*g;
				}
			}

			if ( closestGeyser!=NULL && (!closestGeyser->getType().isRefinery() || !closestGeyser->isBeingConstructed()) )
			{
				// Create refinerey Action
				actionQueue->push(new ConstructBuildingAction(55, closestGeyser->getTilePosition(), BWAPI::Broodwar->self()->getRace().getRefinery()));
			}
		}
	}
}

BuildingManager::~BuildingManager(void)
{
}