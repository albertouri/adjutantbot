#pragma once
#include "action.h"
#include <BWAPI.h>
#include <sstream>

class GatherResourceAction :
	public Action
{
public:
	GatherResourceAction(BWAPI::Unit* worker, BWAPI::Unit* resource);
	~GatherResourceAction(void);
	bool isReady();
	bool isStillValid();
	void execute();
	std::string toString();
};
