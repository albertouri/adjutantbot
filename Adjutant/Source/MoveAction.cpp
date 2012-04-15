#include "MoveAction.h"

MoveAction::MoveAction(BWAPI::Unit* unit,  BWAPI::Position position)
{
	this->unit = unit;
	this->position = BWAPI::Position(position.x(), position.y());
}

MoveAction::MoveAction(BWAPI::Unit* unit, int x, int y)
{
	this->unit = unit;
	this->position = BWAPI::Position(x, y);
}

bool MoveAction::isReady()
{
	return this->unit->exists();
}

bool MoveAction::isStillValid()
{
	return true;
}

void MoveAction::execute()
{
	this->unit->move(this->position);
}

std::string MoveAction::toString()
{
	std::string isStillValidText = (this->isStillValid() ? "T" : "F");
	std::string isReadyText = (this->isReady() ? "T" : "F");
	std::string priorityText, toX, toY;

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	stream.clear();
	stream << this->position.x();
	toX = stream.str();

	stream.clear();
	stream << this->position.y();
	toY = stream.str();

	return "[P:" + priorityText + "]"
		+ "[R:" + isReadyText + "]"
		+ "[V:" + isStillValidText + "]"
		+ " MoveAction"
		+ " " + this->unit->getType().getName() + " to (" + toX + "," + toY + ")";
}


MoveAction::~MoveAction(void) {}