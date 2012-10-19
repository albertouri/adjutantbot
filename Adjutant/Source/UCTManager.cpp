#include "UCTManager.h"

UCTManager::UCTManager(void)
{
}

void UCTManager::evaluate()
{
	//Check for round beginning and end (indicated by messages in-game)
	for each (BWAPI::Event bwEvent in BWAPI::Broodwar->getEvents())
	{
		if (bwEvent.getType() == BWAPI::EventType::ReceiveText)
		{
			if (bwEvent.getText() == "Round Start")
			{
				this->onRoundStart();
			}
			else if (bwEvent.getText() == "Round Complete")
			{
				this->onRoundEnd();
			}
		}
	}
}

void UCTManager::onRoundStart()
{

}

void UCTManager::onRoundEnd()
{

}

UCTManager::~UCTManager(void)
{
}
