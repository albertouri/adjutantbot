#pragma once
#include "ActionQueue.h"
#include <BWAPI.h>
#include <sstream>
#include "Utils.h"

class MoveAction : public Action
{
public:
	MoveAction(BWAPI::Unit* unit, BWAPI::Position position);
	MoveAction::MoveAction(BWAPI::Unit* unit, int x, int y);
	MoveAction::MoveAction(std::vector<BWAPI::Unit*>* unitVector,  BWAPI::Position position);
	MoveAction::MoveAction(std::vector<BWAPI::Unit*>* unitVector, int x, int y);
	~MoveAction(void);
	bool isReady(int minerals, int gas, int supplyRemaining);;
	bool isStillValid();
	void execute();

	bool operator==(const Action &other) const;
	std::string toString();

	std::vector<BWAPI::Unit*>* unitVector;
	BWAPI::Position position;

private:
	bool deleteVectorOnEnd;
};
