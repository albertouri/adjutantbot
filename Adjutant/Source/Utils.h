#pragma once
#include <algorithm>
#include "BWAPI.h"
#include <fstream>
#include "HistoricalUnitInfo.h"
#include <iostream>
#include <sstream>
#include <vector>

class Utils
{
public:
	Utils(void);
	~Utils(void);

	static const bool isDebug = false;

	template <typename T>
	static bool vectorContains(std::vector<T*>* v, T* e)
	{
		if (std::find(v->begin(), v->end(), e) != v->end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool vectorContains(std::vector<BWAPI::UnitType>* v, BWAPI::UnitType e)
	{
		for each (BWAPI::UnitType unitType in (*v))
		{
			if (unitType == e)
			{
				return true;
			}
		}

		return false;
	}

	template <typename T>
	static bool vectorRemoveElement(std::vector<T*>* v, T* e)
	{
		if (std::find(v->begin(), v->end(), e) != v->end())
		{
			v->erase(remove(v->begin(), v->end(), e));
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool vectorRemoveElement(std::vector<BWAPI::UnitType>* v, BWAPI::UnitType e)
	{
		for each (BWAPI::UnitType unitType in (*v))
		{
			if (unitType.getID() == e.getID())
			{
				v->erase(remove(v->begin(), v->end(), e));
				return true;
			}
		}

		return false;
	}

	//Unit belongs to me
	static bool unitIsMine(BWAPI::Unit* unit)
	{
		return unit->getPlayer() == BWAPI::Broodwar->self();
	}

	//Unit is an enemy unit (non-neutral)
	static bool unitIsEnemy(BWAPI::Unit* unit)
	{
		return unit->getPlayer()->isEnemy(BWAPI::Broodwar->self());
	}

	//Get a worker unit that isn't occupied
	static BWAPI::Unit* getFreeWorker(std::vector<BWAPI::Unit*>* workerVector)
	{
		BWAPI::Unit* freeWorker = NULL;

		for each (BWAPI::Unit* worker in (*workerVector))
		{
			if (! worker->isGatheringGas() && ! worker->isCarryingMinerals() && ! worker->isConstructing())
			{
				freeWorker = worker;
				break;
			}
		}

		return freeWorker;
	}

	static bool isValidBuildingLocation(BWAPI::TilePosition tilePosition, BWAPI::UnitType buildingType)
	{
		bool isValidLocation = true;
		bool drawBuildingLocation = true;
		
		//Special case for refineries
		if (buildingType.getID() == BWAPI::UnitTypes::Terran_Refinery)
		{
			return true;
		}

		//check all tiles for building
		for (int i=0; i<buildingType.tileHeight(); i++)
		{
			BWAPI::TilePosition testTile = BWAPI::TilePosition(tilePosition.x(), tilePosition.y());
			testTile.y() += i;

			for (int j=0; j<buildingType.tileWidth(); j++)
			{			
				testTile.x() += 1;

				bool isValidTile = BWAPI::Broodwar->isBuildable(testTile, true)
					&& BWAPI::Broodwar->isExplored(testTile);

				if (! isValidTile)
				{
					isValidLocation = false;
					if (! drawBuildingLocation) {break;}
				}

				if (drawBuildingLocation) 
				{
					BWAPI::Color color = (isValidTile ? BWAPI::Colors::Orange : BWAPI::Colors::Red);
					BWAPI::Broodwar->drawCircleMap(
						BWAPI::Position(testTile).x(), BWAPI::Position(testTile).y(), 
						3, color, true);
				}
			}

			if (! isValidLocation && !drawBuildingLocation) {break;}
		}

		return isValidLocation;
	}

	static BWAPI::Unit* getClosestUnit(BWAPI::Unit* unit, const std::set<BWAPI::Unit*>* otherVector)
	{
		BWAPI::Unit* closestUnit = NULL;
		double minDist = -1;

		for each (BWAPI::Unit* otherUnit in (*otherVector))
		{
			double newDist = otherUnit->getDistance(unit);

			if (closestUnit == NULL || minDist < newDist)
			{
				minDist = newDist;
				closestUnit = otherUnit;
			}
		}

		return closestUnit;
	}

	
	static bool canMakeGivenUnits(BWAPI::UnitType type)
	{
		for each (std::pair<BWAPI::UnitType, int> pair in type.requiredUnits())
		{
			bool pass = false;

			if (BWAPI::Broodwar->self()->completedUnitCount(pair.first) >= pair.second)
			{
				pass = true;
			}

			if (pair.first == BWAPI::UnitTypes::Zerg_Hatchery)
			{
				if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) >= pair.second)
				{
					pass = true;
				}
				else if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) >= pair.second)
				{
					pass = true;
				}
			}

			if (pair.first == BWAPI::UnitTypes::Zerg_Lair && 
				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) >= pair.second)
			{
					pass = true;
			}

			if (pass == false)
			{
				return false;
			}
		}

		if (type.requiredTech() != BWAPI::TechTypes::None)
		{
			if (!BWAPI::Broodwar->self()->hasResearched(type.requiredTech()))
			{
				return false;
			}
		}

		return true;
	}

	static void log(std::string text)
	{
		static int writeCount = 0;
		static int fileCount = 0;

		if (Utils::isDebug)
		{
			std::stringstream fileName = std::stringstream();
			fileName << "AdjutantLog_" << fileCount << ".txt";
			std::ofstream myfile;
			myfile.open (fileName.str().c_str(), std::ios::app);
			myfile << BWAPI::Broodwar->getFrameCount() << ":" << text << std::endl;
			myfile.close();
			
			//Check file size to make sure it doesn't get too big
			if (writeCount > 5000)
			{
				FILE* pFile;
				long size;//in bytes

				fopen_s(&pFile, fileName.str().c_str(),"rb");
				
				if (pFile != NULL)
				{
					fseek (pFile, 0, SEEK_END);
					size=ftell (pFile);
					fclose (pFile);
				}

				//Limit each log file to ~300MB
				if (size > 1024 * 1024 * 300)
				{
					fileCount++;
				}

				writeCount = 0;
			}

			writeCount++;
		}
	}
};
