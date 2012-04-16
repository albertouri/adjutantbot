#pragma once
#include "action.h"
#include <BWAPI.h>
#include <sstream>

class AttackAction : public Action
{
public:
	AttackAction(BWAPI::Unit* unit, BWAPI::Position position);
	AttackAction::AttackAction(BWAPI::Unit* unit, int x, int y);
	AttackAction::AttackAction(std::vector<BWAPI::Unit*>* unitVector,  BWAPI::Position position);
	AttackAction::AttackAction(std::vector<BWAPI::Unit*>* unitVector, int x, int y);
	AttackAction::AttackAction(std::vector<BWAPI::Unit*>* unitVector,  BWAPI::Unit* target);
	AttackAction::AttackAction(BWAPI::Unit* unit,  BWAPI::Unit* target);
	~AttackAction(void);
	bool isReady();
	bool isStillValid();
	void execute();
	std::string toString();

	std::vector<BWAPI::Unit*>* unitVector;
	BWAPI::Position position;
	BWAPI::Unit* target;
private:
	bool deleteVectorOnEnd;
	
};
