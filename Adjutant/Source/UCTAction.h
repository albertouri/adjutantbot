#pragma once

class UCTAction
{
public:
	UCTAction(void);
	~UCTAction(void);

	enum UCTActionType
	{
		AttackAction,
		JoinAction
	}; 

	UCTActionType type;

	double totalReward;
	int visitCount;
};
