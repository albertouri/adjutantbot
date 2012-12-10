#include <iostream>
#include <fstream>
#include <string>
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
			else if (Utils::unitIsMine(unit) && unit->getType().canMove())
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

				file << "MY Type: " << matchUp->myUnit->getType() << ", ";
				file << "TypeName: " << matchUp->myUnitType.getName() << ", ";
				file << "S_HP: " << matchUp->myUnit->getInitialHitPoints() << ", ";
				file << "E_HP: " << matchUp->myUnit->getHitPoints() << " ####### ";

				file << "ENEMY Type: " << matchUp->enemyUnit->getType() << ", ";
				file << "TypeName: " << matchUp->enemyUnitType.getName() << ", ";
				file << "S_HP: " << matchUp->enemyUnit->getInitialHitPoints() << ", ";
				file << "E_HP: " << matchUp->enemyUnit->getHitPoints() << std::endl;
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
