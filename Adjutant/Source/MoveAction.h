#pragma once
#include "action.h"
#include <BWAPI.h>
#include <sstream>

class MoveAction : public Action
{
public:
	MoveAction(BWAPI::Unit* unit, BWAPI::Position position);
	MoveAction::MoveAction(BWAPI::Unit* unit, int x, int y);
	MoveAction::MoveAction(std::vector<BWAPI::Unit*>* unitVector,  BWAPI::Position position);
	MoveAction::MoveAction(std::vector<BWAPI::Unit*>* unitVector, int x, int y);
	~MoveAction(void);
	bool isReady();
	bool isStillValid();
	void execute();
	std::string toString();

	std::vector<BWAPI::Unit*>* unitVector;
	BWAPI::Position position;

private:
	bool deleteVectorOnEnd;
};
