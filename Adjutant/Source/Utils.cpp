#include "Utils.h"

//Init static member variables
int Utils::writeCount = 0;
int Utils::fileCount = 0;
std::ofstream Utils::logFile;

//Unit belongs to me
bool Utils::unitIsMine(BWAPI::Unit* unit)
{
	return unit->getPlayer() == BWAPI::Broodwar->self();
}

//Unit is an enemy unit (non-neutral)
bool Utils::unitIsEnemy(BWAPI::Unit* unit)
{
	return unit->getPlayer()->isEnemy(BWAPI::Broodwar->self());
}

//Get a worker unit that isn't occupied
BWAPI::Unit* Utils::getFreeWorker(std::vector<BWAPI::Unit*>* workerVector)
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

	if (freeWorker = NULL)
	{
		for each (BWAPI::Unit* worker in (*workerVector))
		{
			if (! worker->isConstructing())
			{
				freeWorker = worker;
				break;
			}
		}
	}

	return freeWorker;
}

BWAPI::Unit* Utils::getFreeWorker(std::set<BWAPI::Unit*>* workerSet)
{
	std::vector<BWAPI::Unit*> workerVector(workerSet->begin(), workerSet->end());
	return Utils::getFreeWorker(&workerVector);
}

bool Utils::isValidBuildingLocation(BWAPI::TilePosition tilePosition, BWAPI::UnitType buildingType)
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

BWAPI::Unit* Utils::getClosestUnit(BWAPI::Unit* unit, const std::set<BWAPI::Unit*>* otherVector)
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


bool Utils::canMakeGivenUnits(BWAPI::UnitType type)
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

void Utils::log(std::string text, int level)
{
	std::stringstream fileName = std::stringstream();
	fileName << "AdjutantLog_" << Utils::fileCount << ".txt";

	if (! Utils::logFile.is_open())
	{
		Utils::logFile.open (fileName.str().c_str(), std::ios::app);
	}

	if (level <= Utils::debugLevel)
	{
		Utils::logFile << BWAPI::Broodwar->getFrameCount() << ":" << text << std::endl;
		
		//Check file size to make sure it doesn't get too big
		if (Utils::writeCount > 50000)
		{
			if (Utils::logFile.is_open())
			{
				Utils::logFile.close();
			}

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
				Utils::fileCount++;
			}

			Utils::writeCount = 0;
		}

		Utils::writeCount++;
	}
}

void Utils::onEnd()
{
	if (Utils::logFile.is_open())
	{
		Utils::logFile.close();
	}
}