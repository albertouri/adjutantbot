#include "AttackAction.h"

AttackAction::AttackAction(BWAPI::Unit* unit,  BWAPI::Position position)
{
	this->deleteVectorOnEnd = true;
	this->unitVector = new std::vector<BWAPI::Unit*>();
	this->position = BWAPI::Position(position.x(), position.y());
	this->target = NULL;

	this->unitVector->push_back(unit);
}

AttackAction::AttackAction(BWAPI::Unit* unit, int x, int y)
{
	this->deleteVectorOnEnd = true;
	this->unitVector = new std::vector<BWAPI::Unit*>();
	this->position = BWAPI::Position(x, y);
	this->target = NULL;

	this->unitVector->push_back(unit);
}

AttackAction::AttackAction(std::vector<BWAPI::Unit*>* unitVector,  BWAPI::Position position)
{
	this->deleteVectorOnEnd = false;
	this->unitVector = unitVector;
	this->position = BWAPI::Position(position.x(), position.y());
	this->target = NULL;
}

AttackAction::AttackAction(std::vector<BWAPI::Unit*>* unitVector, int x, int y)
{
	this->deleteVectorOnEnd = true;
	this->unitVector = unitVector;
	this->position = BWAPI::Position(x, y);
	this->target = NULL;
}

AttackAction::AttackAction(BWAPI::Unit* unit,  BWAPI::Unit* target)
{
	this->deleteVectorOnEnd = true;
	this->unitVector = new std::vector<BWAPI::Unit*>();
	this->target = target;
	
	this->unitVector->push_back(unit);
}

AttackAction::AttackAction(std::vector<BWAPI::Unit*>* unitVector,  BWAPI::Unit* target)
{
	this->deleteVectorOnEnd = false;
	this->unitVector = unitVector;
	this->target = target;
}	

bool AttackAction::isReady(int minerals, int gas, int supplyRemaining)
{
	return true;
}

bool AttackAction::isStillValid()
{
	return true;
}

void AttackAction::execute()
{
	for each (BWAPI::Unit* unit in (*this->unitVector))
	{
		if (unit->exists())
		{
			if (this->target != NULL)
			{
				if (this->target->exists())
				{
					unit->attack(target);
				}
			}
			else
			{
				unit->attack(this->position);
			}
		}
	}
}

std::string AttackAction::toString()
{
	std::string isStillValidText = (this->isStillValid() ? "T" : "F");
	std::string priorityText, toX, toY;
	std::string unitText = "";

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	stream.str("");
	stream << this->position.x();
	toX = stream.str();

	stream.clear();
	stream << this->position.y();
	stream.str("");

	
	unitText = this->unitVector->front()->getType().getName();

	if (this->unitVector->size() > 1)
	{
		unitText += "+" + (this->unitVector->size() -1);
	}

	return "[P:" + priorityText+ "]"
		+ "[V:" + isStillValidText + "]"
		+ " AttackAction"
		+ " " + unitText + " to (" + toX + "," + toY + ")";
}

bool AttackAction::operator==(const Action &other) const
{
	if (typeid(other) != typeid(AttackAction)) {return false;}
	AttackAction* otherAction = (AttackAction*)&other;
	bool isSame = true;

	if (this->position != otherAction->position)
	{
		isSame = false;
	}
	else if (this->target != otherAction->target)
	{
		isSame = false;
	}
	else if (this->unitVector->size() != otherAction->unitVector->size())
	{
		isSame = false;
	}
	else
	{
		for each (BWAPI::Unit* unit in (*this->unitVector))
		{
			if (! Utils::vectorContains(otherAction->unitVector, unit))
			{
				isSame = false;
				break;
			}
		}
	}

	return isSame;
}

AttackAction::~AttackAction(void)
{
	if (deleteVectorOnEnd)
	{
		delete this->unitVector;
	}
}