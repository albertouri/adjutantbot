#pragma once
#include "action.h"
#include <BWAPI.h>
#include <sstream>

class MoveAction : public Action
{
public:
	MoveAction(BWAPI::Unit* unit, BWAPI::Position position);
	MoveAction::MoveAction(BWAPI::Unit* unit, int x, int y);
	~MoveAction(void);
	bool isReady();
	bool isStillValid();
	void execute();
	std::string toString();

	BWAPI::Unit* unit;
	BWAPI::Position position;
};
