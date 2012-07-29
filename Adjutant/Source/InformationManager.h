#pragma once
#include <queue>
#include <BWAPI.h>
#include "WorldManager.h"

class InformationManager
{
public:
	InformationManager(void);
	~InformationManager(void);
	void evaluate();
	void manageThreatDetection();
};
