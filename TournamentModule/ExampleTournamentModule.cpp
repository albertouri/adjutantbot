#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <windows.h>
#include <Shlwapi.h>
#include "ExampleTournamentModule.h"
using namespace BWAPI;

bool leader = false;

void ExampleTournamentAI::onStart()
{
  // Set the command optimization level (reduces high APM, size of bloated replays, etc)
  Broodwar->setCommandOptimizationLevel(MINIMUM_COMMAND_OPTIMIZATION);
  Broodwar->setGUI(false);
  Broodwar->setLocalSpeed(0);

  this->roundCount = 0;
  this->maxEventTime = 0;
  this->player1 = Broodwar->getPlayer(0);
  this->player2 = Broodwar->getPlayer(1);
  this->roundInProgress = false;
}

void ExampleTournamentAI::onEnd(bool isWinner)
{

}

void ExampleTournamentAI::onFrame()
{
  int lastEventTime = Broodwar->getLastEventTime();

  if (lastEventTime > maxEventTime)
  {
	  maxEventTime = lastEventTime;
  }

  int p1UnitCount = 0;
  int p2UnitCount = 0;

  for each (BWAPI::Unit* unit in this->player1->getUnits())
  {
	  if (unit->getType().canMove())
	  {
			p1UnitCount++;
	  }
  }

  for each (BWAPI::Unit* unit in this->player2->getUnits())
  {
	  if (unit->getType().canMove())
	  {
			p2UnitCount++;
	  }
  }

	//Check for round beginning and end
	if (this->roundInProgress)
	{
		//Round just ended
		  if (p1UnitCount == 0 || p2UnitCount == 0)
		  {
			this->recordEndRoundStats();
			this->roundInProgress = false;
		  }
	}
	else
	{
		//Round just started
		if (p1UnitCount > 0 && p2UnitCount > 0)
		{
			this->roundInProgress = true;
		}
	}


}

void ExampleTournamentAI::onSendText(std::string text)
{
}

void ExampleTournamentAI::onReceiveText(BWAPI::Player* player, std::string text)
{
}

void ExampleTournamentAI::onPlayerLeft(BWAPI::Player* player)
{
}

void ExampleTournamentAI::onNukeDetect(BWAPI::Position target)
{
}

void ExampleTournamentAI::onUnitDiscover(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitEvade(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitShow(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitHide(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitCreate(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitDestroy(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitMorph(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onUnitRenegade(BWAPI::Unit* unit)
{
}

void ExampleTournamentAI::onSaveGame(std::string gameName)
{
}

void ExampleTournamentAI::onUnitComplete(BWAPI::Unit *unit)
{
}

void ExampleTournamentAI::onPlayerDropped(BWAPI::Player* player)
{
}

bool ExampleTournamentModule::onAction(int actionType, void *parameter)
{
  switch ( actionType )
  {
  case Tournament::SendText:
  case Tournament::Printf:
    return true;
  case Tournament::EnableFlag:
    switch ( *(int*)parameter )
    {
    case Flag::CompleteMapInformation:
    case Flag::UserInput:
      // Disallow these two flags
      return false;
    }
    // Allow other flags if we add more that don't affect gameplay specifically
    return true;
  case Tournament::PauseGame:
  case Tournament::RestartGame:
  case Tournament::ResumeGame:
  case Tournament::SetFrameSkip:
  case Tournament::SetGUI:
  case Tournament::SetLocalSpeed:
  case Tournament::SetMap:
    return false; // Disallow these actions
  case Tournament::LeaveGame:
  case Tournament::ChangeRace:
  case Tournament::SetLatCom:
  case Tournament::SetTextSize:
    return true; // Allow these actions
  case Tournament::SetCommandOptimizationLevel:
    return *(int*)parameter > MINIMUM_COMMAND_OPTIMIZATION; // Set a minimum command optimization level 
                                                            // to reduce APM with no action loss
  default:
    break;
  }
  return true;
}

void ExampleTournamentModule::onFirstAdvertisement()
{
  leader = true;
  Broodwar->sendText("Welcome to " TOURNAMENT_NAME "!");
  Broodwar->sendText("Brought to you by " SPONSORS ".");
}

void ExampleTournamentAI::recordEndRoundStats()
{
	float finalScore = 0;

	for each (BWAPI::Unit* unit in this->player1->getUnits())
	{
		if (unit->getType().canMove())
		{
			// (% of HP/Shields) * (resource cost)
			float healthPercent = ((float)unit->getHitPoints() + (float)unit->getShields()) 
				/ ((float)unit->getType().maxHitPoints() + (float)unit->getType().maxShields());
			float value = (float)unit->getType().mineralPrice() + unit->getType().gasPrice();

			finalScore += healthPercent * value;
		}
	}

	for each (BWAPI::Unit* unit in this->player2->getUnits())
	{
		if (unit->getType().canMove())
		{
			// (% of HP/Shields) * (resource cost)
			float healthPercent = ((float)unit->getHitPoints() + (float)unit->getShields()) 
				/ ((float)unit->getType().maxHitPoints() + (float)unit->getType().maxShields());
			float value = (float)unit->getType().mineralPrice() + unit->getType().gasPrice();

			finalScore -= healthPercent * value;
		}
	}

	std::string outputFile = "SCMatchResults.csv";

	//Record stats to a file
	std::string titleLine = "Round, HostName, HomeRace, AwayName, AwayRace, FinalScore, TotalFrames, Map, TimeStamp, MaxFrameTime, Timestamp";

	time_t timestamp = time(NULL);
	std::stringstream outputLine;
	outputLine << roundCount << ",";
	outputLine << player1->getName() << ",";
	outputLine << player1->getRace().c_str() << ",";
	outputLine << player2->getName() << ",";
	outputLine << player2->getRace().c_str() << ",";
	outputLine << finalScore << ",";
	outputLine << Broodwar->getFrameCount() << ",";
	outputLine << Broodwar->mapFileName() << ",";
	outputLine << maxEventTime << ",";
	outputLine << asctime(gmtime(&timestamp));
	
	//Check for first time writing file
	bool isFirst = false;
	std::ifstream testFile(outputFile.c_str());
	if (testFile.is_open())
	{
		testFile.close();
	}
	else
	{
		isFirst = true;
	}

	std::ofstream resultsFile;
	resultsFile.open (outputFile.c_str(), std::ios::out | std::ios::app);

	if (isFirst)
	{
		resultsFile << titleLine << std::endl;
	}

	resultsFile << outputLine.str();
	resultsFile.close();
	this->roundCount++;
}