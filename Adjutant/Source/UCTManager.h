#pragma once
#include "BWAPI.h"
#include "UCTNode.h"
#include "WorldManager.h"

class UCTManager
{
public:
	UCTManager(void);
	~UCTManager(void);

	void evaluate();

private:
	bool roundInProgress;
	UCTNode* root;

	void onRoundStart();
	void onRoundEnd();
};
