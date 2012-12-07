#pragma once
#include "UCTAction.h"
#include "UCTGroup.h"

class UCTJoinAction : public UCTAction
{
public:
	UCTJoinAction(int myGroupIndex0, int myGroupIndex1, int myGroupIndex2, int myGroupIndex3);
	UCTJoinAction(int myGroupIndex0, int myGroupIndex1, int myGroupIndex2);
	UCTJoinAction(int myGroupIndex0, int myGroupIndex1);
	~UCTJoinAction(void);
	void getCentroid(std::vector<UCTGroup*>* groupVector, int* x, int* y);
	std::vector<UCTGroup*> getGroups(std::vector<UCTGroup*>* allPossibleGroups);

	std::vector<int> groupIdVector;

private:
	void init(int myGroupIndex0, int myGroupIndex1, int myGroupIndex2, int myGroupIndex3);
};
