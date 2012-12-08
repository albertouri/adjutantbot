#include "UCTAction.h"
#include "UCTNode.h"

UCTAction::UCTAction(void)
{
	this->name = "";
	this->resultantNode = NULL;
	this->totalReward = 0;
	this->visitCount = 0;
	this->type = Invalid;
}

double UCTAction::getAverageReward()
{
	if (visitCount == 0)
	{
		return 0;
	}
	else
	{
		return (this->totalReward / this->visitCount);
	}
}

std::string UCTAction::toString()
{
	std::stringstream stream;
	stream << "[" << this->name << "]";
	stream << "[avgR:" << this->getAverageReward() << "]";
	stream << "[n:" << this->visitCount << "]";
	
	if (this->resultantNode != NULL)
	{
		stream << "[next:" << this->resultantNode << "]";
	}

	return stream.str();
}

UCTAction::~UCTAction(void)
{
}
