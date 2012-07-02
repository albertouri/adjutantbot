#include "BuildManager.h"


BuildManager::BuildManager(void)
{
}

void BuildManager::evalute(ActionQueue* actionQueue)
{
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
				if (BWAPI::Broodwar->getFrameCount() > 3000
					&& WorldManager::Instance().myUnitMap[BWAPI::UnitTypes::Terran_Refinery].empty())
				{
					actionQueue->push(new ConstructBuildingAction(40, closestGeyser->getTilePosition(), BWAPI::Broodwar->self()->getRace().getRefinery()));
				}
			}
		}
	}
}

BuildManager::~BuildManager(void)
{
}