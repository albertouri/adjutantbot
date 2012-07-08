#include "BuildTaskComparator.h"

BuildTaskComparator::BuildTaskComparator(void)
{
}

bool BuildTaskComparator::operator() (BuildTask* buildTask1, BuildTask* buildTask2)
{
	//Higher priority has lower number
	if (buildTask1->priority <= buildTask2->priority)
	{
		return false;
	}
	else
	{
		return true;
	}
}

BuildTaskComparator::~BuildTaskComparator(void)
{
}


