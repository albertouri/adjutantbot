#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include "Utils.h"

class Base
{
public:
	Base(BWAPI::Unit* resourceDepot);
	~Base(void);

	BWAPI::Unit* resourceDepot;
	std::set<BWAPI::Unit*> refineryVector;
	BWTA::BaseLocation* baseLocation;

	std::set<BWAPI::Unit*> getMineralWorkers();
	std::set<BWAPI::Unit*> getGasWorkers();

	bool addWorker(BWAPI::Unit* unit);
	bool removeWorker(BWAPI::Unit* unit);
	BWAPI::Unit* removeWorkerNear(BWAPI::Position position);
	BWAPI::Unit* removeWorker();
	int getTotalWorkerCount();	

	bool addRefinery(BWAPI::Unit* unit);
	bool removeRefinery(BWAPI::Unit* unit);
	int getCompletedRefineryCount();

	bool isSaturated();
	bool isMinedOut();
private:
	std::set<BWAPI::Unit*> mineralWorkers;
	std::set<BWAPI::Unit*> gasWorkers;
};
