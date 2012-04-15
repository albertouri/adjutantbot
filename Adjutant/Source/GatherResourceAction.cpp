#include "GatherResourceAction.h"

GatherResourceAction::GatherResourceAction(BWAPI::Unit* worker, BWAPI::Unit* resource)
{
	//TODO: set internal variables
}

bool GatherResourceAction::isReady()
{
	//TODO: add pre-condition if any
	return true;
}

bool GatherResourceAction::isStillValid()
{
	//TODO: assuming action is held in the queue for a long time, use this condition to check if still valid
	return true;
}

void GatherResourceAction::execute()
{
	//TODO: actual BWAPI calls to execute the action
}

std::string GatherResourceAction::toString()
{
	std::string isStillValidText = (this->isStillValid() ? "T" : "F");
	std::string isReadyText = (this->isReady() ? "T" : "F");
	std::string priorityText;

	std::stringstream stream;
	stream << this->priority;
	priorityText = stream.str();

	return "[P:" + priorityText + "]"
		+ "[R:" + isReadyText + "]"
		+ "[V:" + isStillValidText + "]"
		+ " GatherResourceAction"
		+ " " ; //TODO: add extra useful info to action if any
}

GatherResourceAction::~GatherResourceAction(void)
{
	//TODO:De-initialize any dynamic objects... shouldn't be needed
}
