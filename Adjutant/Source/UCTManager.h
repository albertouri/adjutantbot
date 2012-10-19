#pragma once
#include "BWAPI.h"
#include "UCTNode.h"

class UCTManager
{
public:
	UCTManager(void);
	~UCTManager(void);

	void evaluate();

private:
	UCTNode* root;

	void onRoundStart();
	void onRoundEnd();
};
