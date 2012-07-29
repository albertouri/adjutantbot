#pragma once
#include <queue>
#include <BWAPI.h>
#include "WorldManager.h"
#include "BFSBuildingPlacer.h"
#include "ReservedMap.h"

class BuildManager
{
	public:
		BuildManager(void);
		~BuildManager(void);
		void evalute();
	private:
		ReservedMap* reservedMap;
		BFSBuildingPlacer* defaultBuildingPlacer;
};
