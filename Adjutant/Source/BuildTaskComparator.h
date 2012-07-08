#pragma once
#include "BuildTask.h"
class BuildTaskComparator
{
public:
	BuildTaskComparator(void);
	bool operator() (BuildTask* buildTask1, BuildTask* buildTask2);
	~BuildTaskComparator(void);
};
