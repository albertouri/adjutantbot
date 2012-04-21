#pragma once
#include "ActionQueue.h"
#include <BWAPI.h>
#include <sstream>

class GatherResourceAction :
	public Action
{
public:
	GatherResourceAction(BWAPI::Unit* worker, BWAPI::Unit* resource);
	~GatherResourceAction(void);
	bool isReady(int minerals, int gas, int supplyRemaining);;
	bool isStillValid();
	void execute();

	bool operator==(const Action &other) const;
	std::string toString();
};
