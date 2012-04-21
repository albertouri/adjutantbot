#pragma once
#include <string>

class Action
{
public:
	Action(void);
	~Action(void);
	virtual void execute() = 0;
	virtual bool isReady(int minerals, int gas, int supplyRemaining) = 0;
	virtual bool isStillValid() = 0;
	virtual std::string toString() = 0;
	virtual void updateResourceCost(int* minerals, int* gas, int* supplyRemaining);
	virtual bool operator==(const Action &other) const = 0;
	virtual bool operator!=(const Action &other) const;

	int priority;
	int frameCost;
};
