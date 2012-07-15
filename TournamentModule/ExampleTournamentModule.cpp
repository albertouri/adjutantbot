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

  this->maxEventTime = 0;
  this->home = Broodwar->self();
  this->away = Broodwar->enemy();
}

void ExampleTournamentAI::onEnd(bool isWinner)
{
	int status = (isWinner ? 0 : 1);
	recordEndGameStats(status);

	//Create file that tells the automation tool that the game is over
	std::ofstream myfile;
	myfile.open ("C:\\BroodwarAutoMatchup_GameOverFlag.txt");
	myfile << "Game is over";
	myfile.close();
}

void ExampleTournamentAI::onFrame()
{
  // If the elapsed game time has exceeded 1 hour in game time
  if ( Broodwar->getFrameCount() > 86400 ) 
  {
    Broodwar->leaveGame();
	recordEndGameStats(2);
  }
	
  int lastEventTime = Broodwar->getLastEventTime();

/*
	if (Broodwar->getFrameCount() >= 2 && lastEventTime > )
	{
		lastEventTime	
	}
	else if (Broodwar->getFrameCount() >= 10)
	{

	}
	else if (Broodwar->getFrameCount() >= 10)
	{

	}
*/
  if (lastEventTime > maxEventTime)
  {
	  maxEventTime = lastEventTime;
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

void ExampleTournamentAI::recordEndGameStats(int status)
{
	//Temporarily enable complete map info so that we can get the enemy score
	Broodwar->enableFlag(Flag::CompleteMapInformation);
	Player* victor = NULL;
	int homeScore = home->getUnitScore() + home->getKillScore() + home->getBuildingScore() + home->getRazingScore() + home->getCustomScore();
	int awayScore = away->getUnitScore() + away->getKillScore() + away->getBuildingScore() + away->getRazingScore() + away->getCustomScore();
	bool isTimeout = false;
	std::string outputFile = "SCMatchResults.csv";

	if (status == 0)
	{
		victor = home;
	}
	else if (status == 1)
	{
		victor = away;
	}
	else
	{
		isTimeout = true;

		if (homeScore >= awayScore)
		{
			victor = home;
		}
		else
		{
			victor = away;
		}
	}

	//Record stats to a file
	//GameID, HostName, AwayName, VictorName, TotalFrames, isTimeout Map, TimeStamp, HomeScore, AwayScore, MaxFrameTime, Timestamp

	time_t timestamp = time(NULL);
	std::stringstream outputLine;
	
	outputLine << home->getName() << ",";
	outputLine << away->getName() << ",";
	outputLine << victor->getName() << ",";
	outputLine << Broodwar->getFrameCount() << ",";
	outputLine << (isTimeout ? "true" : "false") << ",";
	outputLine << Broodwar->mapFileName() << ",";
	outputLine << homeScore << ",";
	outputLine << awayScore << ",";
	outputLine << maxEventTime << ",";
	outputLine << asctime(gmtime(&timestamp));
	
	std::ofstream myfile;
	myfile.open (outputFile.c_str(), std::ios::out | std::ios::app);
	myfile << outputLine.str();
	myfile.close();
}