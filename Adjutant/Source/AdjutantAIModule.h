#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include <queue>
#include <vector>
#include "Action.h"
#include "ActionComparator.h"
#include "AwarenessModule.h"
#include "MacroModule.h"
#include "TrainUnitAction.h"
#include "Utils.h"
#include "WorldModel.h"

extern bool analyzed;
extern bool analysisJustFinished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

class AdjutantAIModule : public BWAPI::AIModule
{
public:
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player* player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player* player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit* unit);
	virtual void onUnitEvade(BWAPI::Unit* unit);
	virtual void onUnitShow(BWAPI::Unit* unit);
	virtual void onUnitHide(BWAPI::Unit* unit);
	virtual void onUnitCreate(BWAPI::Unit* unit);
	virtual void onUnitDestroy(BWAPI::Unit* unit);
	virtual void onUnitMorph(BWAPI::Unit* unit);
	virtual void onUnitRenegade(BWAPI::Unit* unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit *unit);

	//not part of BWAPI::AIModule
	void drawStats(); 
	void drawQueueStats();
	void drawBullets();
	void drawVisibilityData();
	void drawTerrainData();
	void showPlayers();
	void showForces();

	//Mark the last time we captured the queue to display it to the screen
	int lastQueueCapture;

	bool showBullets;
	bool showStats;
	bool showQueueStats;
	bool showTerrain;
	bool isBotEnabled;
	bool showVisibilityData;
	
	WorldModel* worldModel;
	AwarenessModule* awarenessModule;
	MacroModule* macroModule;

	std::priority_queue<Action*, std::vector<Action*>, ActionComparator> actionQueue;
	std::vector<std::string>* queueTextVector;
};
