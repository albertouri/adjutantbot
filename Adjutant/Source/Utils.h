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

	static void onEnd();

	static const int debugLevel = 0;

	//Unit belongs to me
	static bool unitIsMine(BWAPI::Unit* unit);

	//Unit is an enemy unit (non-neutral)
	static bool unitIsEnemy(BWAPI::Unit* unit);

	//Get a worker unit that isn't occupied
	static BWAPI::Unit* getFreeWorker(std::vector<BWAPI::Unit*>* workerVector);
	static BWAPI::Unit* getFreeWorker(std::set<BWAPI::Unit*>* workerSet);

	static bool isValidBuildingLocation(BWAPI::TilePosition tilePosition, BWAPI::UnitType buildingType);

	static BWAPI::Unit* getClosestUnit(BWAPI::Unit* unit, const std::set<BWAPI::Unit*>* otherVector);
	
	static bool canMakeGivenUnits(BWAPI::UnitType type);

	static void log(std::string text, int level);

	//Functions using templates must be defined in here
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

	template <typename T>
	static bool setContains(const std::set<T*>* set, T* e)
	{
		if (set->find(e) != set->end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	template <typename T>
	static bool setRemoveElement(std::set<T*>* set, T* e)
	{
		if (set->erase(e) > 0)
		{
			return true;
		}
		else
		{
			return false;
		}
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

private:
	static int writeCount;
	static int fileCount;
	static std::ofstream logFile;
};
