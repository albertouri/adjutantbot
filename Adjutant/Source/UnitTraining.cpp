#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <iomanip>
#include "UnitTraining.h"
#include "AdjutantAIModule.h"

UnitTraining::UnitTraining(void)
{
	isTrainerInitialized = false;
}

void UnitTraining::evalute()
{
	// Should run the first time only
	if(!isTrainerInitialized)
	{
		this->myUnitVector = new std::vector<BWAPI::Unit*>();

		// Get list of my units
		for each (BWAPI::Unit* unit in BWAPI::Broodwar->getAllUnits())
		{	
			/*	Get Hero Trigger
			*	Terran -> Jim Raynor [Hero_Jim_Raynor_Marine], 
			*	Protoss -> Zeratul [Hero_Zeratul], 
			*	Zerg -> Devouring One [Hero_Devouring_One]
			*/
			if (unit->getType() == BWAPI::UnitTypes::Hero_Jim_Raynor_Marine)
			{
				this->heroTrigger = unit;
			}
			// Find closes enemy to each of my units
			else if (this->heroTrigger != NULL 
					&& this->heroTrigger->exists() 
					&& Utils::unitIsMine(unit) && unit->getType().canMove())
			{
				this->myUnitVector->push_back(unit);

				// Create std::map<BWAPI::Unit*, MatchUp*> matchUpMap; Object
				BWAPI::Unit* closestEnemy = Utils::getClosestMovableUnit(unit, &WorldManager::Instance().enemy->getUnits());
				if(closestEnemy != NULL)
				{
					isTrainerInitialized = true;
					MatchUp* matchUp = new MatchUp(unit, closestEnemy);
					this->matchUpMap.insert(std::make_pair(unit, matchUp));
				}
			}
		}
	}
	else 
	{
		// if Hero_Jim_Raynor_Marine is dead then the round is over
		// so cycle through Array of Battle objects 
		if (!this->heroTrigger->exists())
		{
			isTrainerInitialized = false;
			std::ofstream file;
			file.open ("AdjutResultExample.txt", std::ios::app);

			std::string roundType;
			for each (BWAPI::Unit* unit in (*this->myUnitVector))
			{
				MatchUp* matchUp = this->matchUpMap.find(unit)->second;
				roundType = matchUp->myUnitType.getName();

				int myUnitInitial = matchUp->myUnitInitialHitPoints;
				int myUnitFinal = matchUp->myUnit->getHitPoints();
				int enemyUnitInitial = matchUp->enemyUnitInitialHitPoints;
				int enemyUnitFinal = matchUp->enemyUnit->getHitPoints();

				int enemyHP = enemyUnitInitial - enemyUnitFinal;
				if(enemyHP <= 0)
				{
					enemyHP = 1;
				}
				int myHP = myUnitInitial - myUnitFinal;
				if(myHP <= 0)
				{
					myHP = 1;
				}

				float matchUpRatio = (float) enemyHP / myHP;

				file << "MY Type:," << matchUp->myUnitType << ",";
				file << "TypeName:," << matchUp->myUnitType.getName() << ",";

				file << "ENEMY Type:," << matchUp->enemyUnitType << ",";
				file << "TypeName:," << matchUp->enemyUnitType.getName() << ",";

				file << "RATIO1:," << std::setprecision(2) << std::fixed << matchUpRatio << std::endl;
			}

			file << "END OF ROUND. " << roundType << "\n";
			file.close();

			this->myUnitVector->clear();
		}
	}

	Utils::log("Leaving UnitTraining", 1);
}

UnitTraining::~UnitTraining(void)
{
	delete this->myUnitVector;
}
