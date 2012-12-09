#include "UCTJoinAction.h"

UCTJoinAction::UCTJoinAction(int myGroupIndex0, int myGroupIndex1, int myGroupIndex2, int myGroupIndex3)
{
	this->init(myGroupIndex0, myGroupIndex1, myGroupIndex2, myGroupIndex3);
}

UCTJoinAction::UCTJoinAction(int myGroupIndex0, int myGroupIndex1, int myGroupIndex2)
{
	this->init(myGroupIndex0, myGroupIndex1, myGroupIndex2, -1);
}

UCTJoinAction::UCTJoinAction(int myGroupIndex0, int myGroupIndex1)
{
	this->init(myGroupIndex0, myGroupIndex1, -1, -1);
}

void UCTJoinAction::init(int myGroupIndex0, int myGroupIndex1, int myGroupIndex2, int myGroupIndex3)
{
	this->type = JoinAction;
	std::stringstream stream;

	if (myGroupIndex0 != -1)
	{
		this->groupIdVector.push_back(myGroupIndex0);
		stream << myGroupIndex0;
	}

	if (myGroupIndex1 != -1)
	{
		this->groupIdVector.push_back(myGroupIndex1);
		stream << " <-> " << myGroupIndex1;
	}

	if (myGroupIndex2 != -1)
	{
		this->groupIdVector.push_back(myGroupIndex2);
		stream << " <-> " << myGroupIndex2;
	}

	if (myGroupIndex3 != -1)
	{
		this->groupIdVector.push_back(myGroupIndex3);
		stream << " <-> " << myGroupIndex3;
	}

	this->name = stream.str();
}

void UCTJoinAction::getCentroid(std::vector<UCTGroup*>* allGroups, int* x, int* y)
{
	int centroidX = 0;
	int centroidY = 0;

	std::vector<UCTGroup*> groupVector = this->getGroups(allGroups);

	for each (UCTGroup* group in groupVector)
	{
		centroidX += group->positionX;
		centroidY += group->positionY;
	}

	centroidX /= groupVector.size();
	centroidY /= groupVector.size();

	*x = centroidX;
	*y = centroidY;
}

std::vector<UCTGroup*> UCTJoinAction::getGroups(std::vector<UCTGroup*>* allPossibleGroups)
{
	std::vector<UCTGroup*> groupVector;

	for each (int groupId in this->groupIdVector)
	{
		groupVector.push_back(Utils::getGroupById(allPossibleGroups, groupId));
	}

	return groupVector;
}

UCTJoinAction::~UCTJoinAction(void)
{
}
