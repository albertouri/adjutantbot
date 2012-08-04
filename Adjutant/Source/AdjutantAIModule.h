#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <queue>
#include <vector>
#include <time.h>
#include "InformationManager.h"
#include "UnitManager.h"
#include "BuildManager.h"
#include "ScoutingManager.h"
#include "MilitaryManager.h"
#include "Threat.h"
#include "Timer.h"
#include "Utils.h"
#include "WorldManager.h"

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
	void drawBuildOrder();
	void drawArmies();
	void showPlayers();
	void showForces();
	

	bool isBotEnabled;
	bool showBuildOrder;
	bool showGeneralInfo;
	bool showBullets;
	bool showStats;
	bool showTerrain;
	bool showArmies;
	bool showVisibilityData;
	
	InformationManager* informationManager;
	UnitManager* unitManager;
	BuildManager* buildManager;
	ScoutingManager* scoutingManager;
	MilitaryManager* militaryManager;
};
