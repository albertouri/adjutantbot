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

	BuildOrder* marineTank;
};
