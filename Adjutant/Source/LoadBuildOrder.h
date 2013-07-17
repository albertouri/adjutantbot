#pragma once
#include "BuildOrder.h"

class LoadBuildOrder
{
public:
	static LoadBuildOrder& Instance()
	{
		static LoadBuildOrder instance;
		return instance;
	}

	LoadBuildOrder(void);
	~LoadBuildOrder(void);
	BuildOrder* getBuildOrder();

	std::map<std::string, BuildOrder*> buildOrderMap;

	BuildOrder* marineTank;
};
