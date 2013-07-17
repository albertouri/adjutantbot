#include "LoadBuildOrder.h"

LoadBuildOrder::LoadBuildOrder(void)
{
	//-------------------------------------------------------------------------
	// MarineTank Build
	//-------------------------------------------------------------------------
	this->buildOrderMap["MarineTank"] = new BuildOrder("MarineTank Build");
	this->marineTank = this->buildOrderMap["MarineTank"];
	marineTank->setSupplyLimit(BuildOrder::Early, 30*2);
	marineTank->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Marine, 6);
	marineTank->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Medic, 2);
	marineTank->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Firebat, 1);
	marineTank->add(BuildOrder::Early, BWAPI::UpgradeTypes::U_238_Shells);
	marineTank->add(BuildOrder::Early, BWAPI::TechTypes::Stim_Packs);
	
	marineTank->setSupplyLimit(BuildOrder::Mid, 80*2);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Marine, 12);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Medic, 4);
	marineTank->add(BuildOrder::Mid, BWAPI::TechTypes::Tank_Siege_Mode);
	marineTank->add(BuildOrder::Mid, BWAPI::TechTypes::Stim_Packs);
	marineTank->add(BuildOrder::Mid, BWAPI::UpgradeTypes::U_238_Shells);
	marineTank->add(BuildOrder::Mid, BWAPI::UpgradeTypes::Terran_Infantry_Weapons);

	marineTank->setSupplyLimit(BuildOrder::Late, 220*2);
	marineTank->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Marine, 8);
	marineTank->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
	marineTank->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Medic, 2);
	marineTank->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Science_Vessel, 1);
	marineTank->add(BuildOrder::Late, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons);
	marineTank->add(BuildOrder::Late, BWAPI::UpgradeTypes::Terran_Vehicle_Plating);
	marineTank->add(BuildOrder::Late, BWAPI::TechTypes::Tank_Siege_Mode);
	marineTank->add(BuildOrder::Late, BWAPI::TechTypes::Stim_Packs);
	marineTank->add(BuildOrder::Late, BWAPI::UpgradeTypes::U_238_Shells);

	//-------------------------------------------------------------------------
	// QuickMech Build
	//-------------------------------------------------------------------------
	this->buildOrderMap["QuickMech"] = new BuildOrder("QuickMech Build");
	this->buildOrderMap["QuickMech"]->setSupplyLimit(BuildOrder::Early, 20*2);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Marine, 1);
	
	this->buildOrderMap["QuickMech"]->setSupplyLimit(BuildOrder::Mid, 80*2);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Vulture, 5);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Goliath, 3);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Mid, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Mid, BWAPI::UpgradeTypes::Terran_Vehicle_Plating);

	this->buildOrderMap["QuickMech"]->setSupplyLimit(BuildOrder::Late, 220*2);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Vulture, 5);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Goliath, 3);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UnitTypes::Terran_Science_Vessel, 1);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UpgradeTypes::Charon_Boosters);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons);
	this->buildOrderMap["QuickMech"]->add(BuildOrder::Late, BWAPI::UpgradeTypes::Terran_Vehicle_Plating);
}

BuildOrder* LoadBuildOrder::getBuildOrder()
{
	return this->marineTank;
}

LoadBuildOrder::~LoadBuildOrder(void)
{
	delete this->marineTank;
}
