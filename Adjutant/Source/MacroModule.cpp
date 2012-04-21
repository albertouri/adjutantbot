#include "MacroModule.h"

MacroModule::MacroModule(void)
{
}

void MacroModule::evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue)
{
	BuildingManager::evalute(worldModel, actionQueue);
	//examine world model
	//add actions to actionQueue based on current state

	//manually assigning idle workers to mine
	//TODO:Update to use worldModel and actions instead
	BWAPI::Unit* cc = NULL;
	BWAPI::Unit* refinery = NULL;
	BWAPI::Unit* closestGeyser=NULL;
	BWAPI::Unit* closestMineral=NULL;

	int gasWorkers = 0;
	int mineralWorkers = 0;

	// Build world dynamics for meterics and understanding
	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		if ((*unit)->getType().isWorker()) 
		{
			if ((*unit)->isGatheringMinerals() || (*unit)->isCarryingMinerals()) { mineralWorkers++; }
			if ((*unit)->isGatheringGas() || (*unit)->isCarryingGas()) { gasWorkers++; }
		}

		// Refinery
		else if ((*unit)->getType().isRefinery()) 
		{
			refinery = (*unit);
		}

		// Command Center
		else if ((*unit)->getType().isResourceDepot())
		{
			cc = (*unit);

			// Find closest Geysers
			for(std::set<BWAPI::Unit*>::iterator g=BWAPI::Broodwar->getGeysers().begin();g!=BWAPI::Broodwar->getGeysers().end();g++)
			{
				if (closestGeyser==NULL || (*unit)->getDistance(*g) < (*unit)->getDistance(closestGeyser))
				{
					closestGeyser=*g;
				}
			}
			
			for(std::set<BWAPI::Unit*>::iterator m=BWAPI::Broodwar->getMinerals().begin();m!=BWAPI::Broodwar->getMinerals().end();m++)
			{
				if (closestMineral==NULL || (*unit)->getDistance(*m) < (*unit)->getDistance(closestMineral))
				{
					closestMineral=*m;
				}
			}
		}
	}

	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		if ((*unit)->getType().isWorker())
		{
			if ( (*unit)->isIdle() )
			{
				// 6 Mineral workers, 3 Gas Workers
				if (mineralWorkers >= 6 && gasWorkers <= 3 
						&& closestGeyser!=NULL && closestGeyser->getType().isRefinery() && closestGeyser->isCompleted())
				{	
					// Send to refinery for gas gathering
					(*unit)->rightClick(closestGeyser);
					gasWorkers++;
				}
				else if (closestMineral!=NULL)
				{
					(*unit)->rightClick(closestMineral);
					mineralWorkers++;
				}
			}
			// Worker is gathering something
			else if ( !(*unit)->isIdle() && !(*unit)->isConstructing()) // Else back to the mines with the worker!
			{		
				if (mineralWorkers > 5 && gasWorkers < 3 && refinery!=NULL && !refinery->isBeingConstructed())
				{	
					(*unit)->rightClick(refinery);
					gasWorkers++;
				}
			}
		}
	}

	if (! cc->isTraining() && BWAPI::Broodwar->self()->minerals() >= 50)
	{
		//Initialize new action
		actionQueue->push(new TrainUnitAction(50, cc, BWAPI::Broodwar->self()->getRace().getWorker()));
	}

	//TODO: Add logic for constructing supply depots and other buildings
/*	if (! cc->isTraining() && BWAPI::Broodwar->self()->minerals() >= 100)
	{
		//Initialize new action
		actionQueue->push(new TrainUnitAction(50, cc, BWAPI::Broodwar->self()->getRace().getWorker()));
	}
*/
}

MacroModule::~MacroModule(void)
{
}
