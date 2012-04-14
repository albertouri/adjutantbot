#pragma once

class Action
{
public:
	Action(void);
	~Action(void);
	virtual void execute() = 0;
	virtual bool isReady() = 0;
	virtual bool isStillValid() = 0;
	int priority;
	int cost;
};
