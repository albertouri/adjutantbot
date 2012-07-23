#include "BuildOrder.h"

BuildOrder::BuildOrder(std::string name)
{
	this->unitsVector = std::vector<BuildOrderUnits*>();
	this->unitsVector.push_back(new BuildOrderUnits()); //Early
	this->unitsVector.push_back(new BuildOrderUnits()); //Mid
	this->unitsVector.push_back(new BuildOrderUnits()); //Late
	this->currentPhase = BuildOrder::Early;
	
	this->name = name;
}

void BuildOrder::add(BuildOrder::GamePhase phase, BWAPI::UnitType unitType, int ratio)
{
	
	this->unitsVector[phase]->setUnitRatio(unitType, ratio);
}

void BuildOrder::add(BuildOrder::GamePhase phase, BWAPI::TechType techType)
{
	
	this->unitsVector[phase]->techTypeVector.push_back(techType);
}


void BuildOrder::add(BuildOrder::GamePhase phase, BWAPI::UpgradeType upgradeType)
{
	
	this->unitsVector[phase]->upgradeTypeVector.push_back(upgradeType);
}

void BuildOrder::setSupplyLimit(BuildOrder::GamePhase phase, int limit)
{
	this->unitsVector[phase]->supplyLimit = limit;
}

BuildOrderUnits* BuildOrder::getCurrentUnits()
{
	return this->unitsVector[this->currentPhase];
}

void BuildOrder::checkForTransition()
{
	if (BWAPI::Broodwar->self()->supplyUsed() >= this->unitsVector[this->currentPhase]->supplyLimit
		&&  this->currentPhase < BuildOrder::Late)
	{
		switch (this->currentPhase)
		{
			case BuildOrder::Early:
				this->currentPhase = BuildOrder::Mid;
				break;
			case BuildOrder::Mid:
				this->currentPhase = BuildOrder::Late;
				break;
			case BuildOrder::Late:
			default:
				break;
		}

	}
}

BuildOrder::~BuildOrder(void)
{
	for each (BuildOrderUnits* bou in this->unitsVector)
	{
		delete bou;
	}
}
