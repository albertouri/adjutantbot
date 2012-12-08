#pragma once
#include <sstream>

//Forward declratation of UCTNode class in order to avoid circular dependency
class UCTNode;

class UCTAction
{
public:
	UCTAction(void);
	~UCTAction(void);
	double getAverageReward();
	std::string toString();

	enum UCTActionType
	{
		Invalid,
		AttackAction,
		JoinAction
	}; 

	UCTNode* resultantNode;
	UCTActionType type;
	std::string name;
	double totalReward;
	int visitCount;
};
