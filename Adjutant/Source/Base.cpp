#include "Base.h"

Base::Base(BWAPI::Unit* resourceDepot)
{
	this->resourceDepot = resourceDepot;
	this->baseLocation = BWTA::getNearestBaseLocation(resourceDepot->getPosition());;
}

std::set<BWAPI::Unit*> Base::getMineralWorkers()
{
	return this->mineralWorkers;
}

std::set<BWAPI::Unit*> Base::getGasWorkers()
{
	return this->gasWorkers;
}

bool Base::addWorker(BWAPI::Unit* unit)
{
	if (this->mineralWorkers.size() > 6
		&& this->getCompletedRefineryCount() > 0
		&& this->gasWorkers.size() < (unsigned int)this->getCompletedRefineryCount() * 3)
	{
		//Add worker to gas
		for each (BWAPI::Unit* refinery in this->refineryVector)
		{
			int workerCount = 0;

			for each (BWAPI::Unit* gasWorker in this->getGasWorkers())
			{
				if (gasWorker->getTarget() == refinery) {workerCount++;}
			}

			if (workerCount < 3)
			{
				unit->gather(refinery);
				break;
			}
		}

		this->gasWorkers.insert(unit);
	}
	else if (this->baseLocation->minerals() > 0)
	{
		//Add worker to minerals
		BWAPI::Unit* targetMineral = NULL;

		for each (BWAPI::Unit* mineral in this->baseLocation->getMinerals())
		{
			bool isBeingGathered = false;

			for each (BWAPI::Unit* worker in this->mineralWorkers)
			{
				if (worker->getTarget() == mineral)
				{
					isBeingGathered = true;
				}
			}

			if (! isBeingGathered)
			{
				targetMineral = mineral;
				break;
			}
		}

		//All taken, pick randomly
		if (targetMineral == NULL)
		{
			int choice = rand() % this->baseLocation->getMinerals().size(); //0 to (size - 1)
			std::set<BWAPI::Unit*>::const_iterator it(this->baseLocation->getMinerals().begin());
			std::advance(it,choice);
			targetMineral = (*it);
		}
		
		unit->gather(targetMineral);

		this->mineralWorkers.insert(unit);
	}
	else
	{
		return false; //No minerals, no more spots for gas
	}

	return true;
}

bool Base::removeWorker(BWAPI::Unit* unit)
{
	if (Utils::setRemoveElement(&this->mineralWorkers, unit))
	{
		return true;
	}
	else if (Utils::setRemoveElement(&this->gasWorkers, unit))
	{
		return true;
	}
	else
	{
		return false; //Worker not part of any base
	}
}

int Base::getTotalWorkerCount()
{
	return this->mineralWorkers.size() + this->gasWorkers.size();
}

bool Base::addRefinery(BWAPI::Unit* unit)
{
	for each (BWAPI::Unit* geyser in this->baseLocation->getGeysers())
	{
		if (unit->getPosition() == geyser->getPosition())
		{
			this->refineryVector.insert(unit);

			//Unit will automatically start mining gas before it even becomes
			//idle, so transfer it to gas now
			BWAPI::Unit* builder = unit->getBuildUnit();

			if (builder != NULL && builder->exists())
			{
				this->removeWorker(unit);
				this->gasWorkers.insert(unit);
			}
		}
	}

	return false;
}

bool Base::removeRefinery(BWAPI::Unit* unit)
{
	return Utils::setRemoveElement(&this->refineryVector, unit);
}

bool Base::isMinedOut()
{
	return (this->baseLocation->gas() == 0 && this->baseLocation->minerals() == 0);
}

bool Base::isSaturated()
{
	return (unsigned int) this->getTotalWorkerCount() >= 
		(this->baseLocation->getMinerals().size() * 2) 
		+ (this->baseLocation->getGeysers().size() * 3);
}

int Base::getCompletedRefineryCount()
{
	int count = 0;
	
	for each (BWAPI::Unit* refinery in this->refineryVector)
	{
		if (refinery->isCompleted())
		{
			count++;
		}
	}

	return count;
}

Base::~Base(void)
{
}
