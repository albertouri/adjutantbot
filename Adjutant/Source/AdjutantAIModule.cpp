#include "AdjutantAIModule.h"
using namespace BWAPI;

bool analyzed;
bool analysisJustFinished;
BWTA::Region* home;
BWTA::Region* enemy_base;

void AdjutantAIModule::onStart()
{
	Broodwar->sendText("Hello world!");
	Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
	
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//read map information into BWTA so terrain analysis can be done in another thread
	BWTA::readMap();
	analyzed=false;
	analysisJustFinished=false;

	showBullets=false;
	showVisibilityData=false;
	isBotEnabled=true;
	showStats=true;

	if (Broodwar->isReplay())
	{
		Broodwar->printf("The following players are in this replay:");
		for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
		{
			if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
			{
				Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
			}
		}
	}
	else
	{
		Broodwar->printf("The match up is %s v %s",
			Broodwar->self()->getRace().getName().c_str(),
			Broodwar->enemy()->getRace().getName().c_str());

		if (! (Broodwar->self()->getRace().getName() == Races::Terran.getName()))
		{
			Broodwar->printf("The Adjutant bot can only play as Terran");
			isBotEnabled = false;
		}

		if (isBotEnabled)
		{
			/*
			//send each worker to the mineral field that is closest to it
			for(std::set<Unit*>::const_iterator unit=Broodwar->self()->getUnits().begin();unit!=Broodwar->self()->getUnits().end();unit++)
			{
				if ((*unit)->getType().isWorker())
				{
					Unit* closestMineral=NULL;
					for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
					{
						if (closestMineral==NULL || (*unit)->getDistance(*m) < (*unit)->getDistance(closestMineral))
							closestMineral=*m;
					}
					if (closestMineral!=NULL)
						(*unit)->rightClick(closestMineral);
				}
				else if ((*unit)->getType().isResourceDepot())
				{
					//if this is a center, tell it to build the appropiate type of worker
					if ((*unit)->getType().getRace()!=Races::Zerg)
					{
						(*unit)->train(Broodwar->self()->getRace().getWorker());
					}
					else //if we are Zerg, we need to select a larva and morph it into a drone
					{
						std::set<Unit*> myLarva=(*unit)->getLarva();
						if (myLarva.size()>0)
						{
							Unit* larva=*myLarva.begin();
							larva->morph(UnitTypes::Zerg_Drone);
						}
					}
				}
			}
			*/
		}
	}
}

void AdjutantAIModule::onEnd(bool isWinner)
{
	if (isWinner)
	{
		//log win to file
	}
}

void AdjutantAIModule::onFrame()
{
	if (showVisibilityData) {drawVisibilityData();}
	if (showBullets) {drawBullets();}	
	if (showStats) { drawStats();}
	if (analyzed && isShowTerrain) {drawTerrainData();}
	if (Broodwar->isReplay()) {return;}

	if (isBotEnabled)
	{
		if (Broodwar->getFrameCount()%5==0)
		{
			Unit* cc = NULL;

			//manually assigning idle workers to mine
			for(std::set<Unit*>::const_iterator unit=Broodwar->self()->getUnits().begin();unit!=Broodwar->self()->getUnits().end();unit++)
			{
				if ((*unit)->getType().isWorker() && (*unit)->isIdle())
				{
							Unit* closestMineral=NULL;
					for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
					{
						if (closestMineral==NULL || (*unit)->getDistance(*m) < (*unit)->getDistance(closestMineral))
							closestMineral=*m;
					}
					if (closestMineral!=NULL)
						(*unit)->rightClick(closestMineral);
				}

				if ((*unit)->getType().isResourceDepot())
				{
					cc = (*unit);
				}
			}

			if (! cc->isTraining() && Broodwar->self()->minerals() >= 50)
			{
				Action* a = new TrainUnitAction(50, cc, &Broodwar->self()->getRace().getWorker());
				//Action* a = new Action();
				actionQueue.push(a);
			}

			while(! actionQueue.empty())
			{
				Action* action = actionQueue.top();
				

				if (action->isReady())
				{
					action->execute();
				}

				actionQueue.pop();
			}
		}

		if (analyzed && Broodwar->getFrameCount()%30==0)
		{
			//order one of our workers to guard our chokepoint.
			for(std::set<Unit*>::const_iterator unit=Broodwar->self()->getUnits().begin();unit!=Broodwar->self()->getUnits().end();unit++)
			{
				if ((*unit)->getType().isWorker())
				{
					//get the chokepoints linked to our home region
					std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();
					double min_length=10000;
					BWTA::Chokepoint* choke=NULL;

					//iterate through all chokepoints and look for the one with the smallest gap (least width)
					for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++)
					{
						double length=(*c)->getWidth();
						if (length<min_length || choke==NULL)
						{
							min_length=length;
							choke=*c;
						}
					}

					//order the worker to move to the center of the gap
					(*unit)->rightClick(choke->getCenter());
					break;
				}
			}
		}
	}



	if (analysisJustFinished)
	{
		Broodwar->printf("Finished analyzing map.");
		analysisJustFinished=false;
	}
}

void AdjutantAIModule::onSendText(std::string text)
{
	if (text=="/show bullets")
	{
		showBullets = !showBullets;
	}
	else if (text=="/enable bot")
	{
		isBotEnabled = !isBotEnabled;
	}
	else if (text=="/show stats")
	{
		showStats = !showStats;
	}
	else if (text=="/show terrain")
	{
		isShowTerrain = !isShowTerrain;
	} 
	else if (text=="/show players")
	{
		showPlayers();
	} 
	else if (text=="/show forces")
	{
		showForces();
	} 
	else if (text=="/show visibility")
	{
		showVisibilityData=!showVisibilityData;
	} 
	else if (text=="/analyze")
	{
		if (analyzed == false)
		{
			Broodwar->printf("Analyzing map... this may take a minute");
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
		}
	} 
	else
	{
		Broodwar->printf("You typed '%s'!",text.c_str());
		Broodwar->sendText("%s",text.c_str());
	}
}

void AdjutantAIModule::onReceiveText(BWAPI::Player* player, std::string text)
{
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void AdjutantAIModule::onPlayerLeft(BWAPI::Player* player)
{
	Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void AdjutantAIModule::onNukeDetect(BWAPI::Position target)
{
	if (target!=Positions::Unknown)
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
	else
		Broodwar->printf("Nuclear Launch Detected");
}

void AdjutantAIModule::onUnitDiscover(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
		Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void AdjutantAIModule::onUnitEvade(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
		Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void AdjutantAIModule::onUnitShow(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
		Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void AdjutantAIModule::onUnitHide(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
		Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void AdjutantAIModule::onUnitCreate(BWAPI::Unit* unit)
{
	if (Broodwar->getFrameCount()>1)
	{
		if (!Broodwar->isReplay())
			Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
		else
		{
			/*if we are in a replay, then we will print out the build order
			(just of the buildings, not the units).*/
			if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
			{
				int seconds=Broodwar->getFrameCount()/24;
				int minutes=seconds/60;
				seconds%=60;
				Broodwar->sendText("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
			}
		}
	}
}

void AdjutantAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
		Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void AdjutantAIModule::onUnitMorph(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay())
		Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	else
	{
		/*if we are in a replay, then we will print out the build order
		(just of the buildings, not the units).*/
		if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
		{
			int seconds=Broodwar->getFrameCount()/24;
			int minutes=seconds/60;
			seconds%=60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
		}
	}
}

void AdjutantAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay())
		Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

void AdjutantAIModule::onSaveGame(std::string gameName)
{
	Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();

	//self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
	{
		home			 = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}
	//enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
	{
		enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}
	analyzed	 = true;
	analysisJustFinished = true;
	return 0;
}

void AdjutantAIModule::drawStats()
{
	std::set<Unit*> myUnits = Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
	for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
	{
		if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
		{
			unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
		}
		unitTypeCounts.find((*i)->getType())->second++;
	}
	int line=1;
	for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
	{
		Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
		line++;
	}
}

void AdjutantAIModule::drawBullets()
{
	std::set<Bullet*> bullets = Broodwar->getBullets();
	for(std::set<Bullet*>::iterator i=bullets.begin();i!=bullets.end();i++)
	{
		Position p=(*i)->getPosition();
		double velocityX = (*i)->getVelocityX();
		double velocityY = (*i)->getVelocityY();
		if ((*i)->getPlayer()==Broodwar->self())
		{
			Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
			Broodwar->drawTextMap(p.x(),p.y(),"\x07%s",(*i)->getType().getName().c_str());
		}
		else
		{
			Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
			Broodwar->drawTextMap(p.x(),p.y(),"\x06%s",(*i)->getType().getName().c_str());
		}
	}
}

void AdjutantAIModule::drawVisibilityData()
{
	for(int x=0;x<Broodwar->mapWidth();x++)
	{
		for(int y=0;y<Broodwar->mapHeight();y++)
		{
			if (Broodwar->isExplored(x,y))
			{
				if (Broodwar->isVisible(x,y))
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
				else
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
			}
			else
				Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
		}
	}
}

void AdjutantAIModule::drawTerrainData()
{
	//we will iterate through all the base locations, and draw their outlines.
	for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
	{
		TilePosition p=(*i)->getTilePosition();
		Position c=(*i)->getPosition();

		//draw outline of center location
		Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

		//draw a circle at each mineral patch
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++)
		{
			Position q=(*j)->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
		}

		//draw the outlines of vespene geysers
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
		{
			TilePosition q=(*j)->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
			Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
	{
		BWTA::Polygon p=(*r)->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
	{
		for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
		{
			Position point1=(*c)->getSides().first;
			Position point2=(*c)->getSides().second;
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
		}
	}
}

void AdjutantAIModule::showPlayers()
{
	std::set<Player*> players=Broodwar->getPlayers();
	for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
	{
		Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
	}
}

void AdjutantAIModule::showForces()
{
	std::set<Force*> forces=Broodwar->getForces();
	for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
	{
		std::set<Player*> players=(*i)->getPlayers();
		Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
		for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
		{
			Broodwar->printf("	- Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
		}
	}
}

void AdjutantAIModule::onUnitComplete(BWAPI::Unit *unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
		Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}
