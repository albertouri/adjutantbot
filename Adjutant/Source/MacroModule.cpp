#include "MacroModule.h"

MacroModule::MacroModule(void)
{
}

void MacroModule::evalute(WorldModel* worldModel, std::priority_queue<Action*, std::vector<Action*>, ActionComparator>* actionQueue)
{
	//examine world model
	//add actions to actionQueue based on current state

	//manually assigning idle workers to mine
	//TODO:Update to use worldModel and actions instead
	BWAPI::Unit* cc = NULL;

	for(std::set<BWAPI::Unit*>::const_iterator unit=BWAPI::Broodwar->self()->getUnits().begin();unit!=BWAPI::Broodwar->self()->getUnits().end();unit++)
	{
		if ((*unit)->getType().isWorker() && (*unit)->isIdle())
		{
			BWAPI::Unit* closestMineral=NULL;
			for(std::set<BWAPI::Unit*>::iterator m=BWAPI::Broodwar->getMinerals().begin();m!=BWAPI::Broodwar->getMinerals().end();m++)
			{
				if (closestMineral==NULL || (*unit)->getDistance(*m) < (*unit)->getDistance(closestMineral))
					closestMineral=*m;
			}
			if (closestMineral!=NULL)
				(*unit)->rightClick(closestMineral);
		}

		if ((*unit)->getType().isResourceDepot())
		{
			cc = (*unit);
		}
	}

	if (! cc->isTraining() && BWAPI::Broodwar->self()->minerals() >= 50)
	{
		//Initialize new action
		actionQueue->push(new TrainUnitAction(50, cc, BWAPI::Broodwar->self()->getRace().getWorker()));
	}

	//TODO: Add logic for constructing supply depots and other buildings
}

MacroModule::~MacroModule(void)
{
}
