#include "LoadBuildOrder.h"

LoadBuildOrder::LoadBuildOrder(void)
{
	this->marineTank = new BuildOrder("MarineTank Build");
	marineTank->setSupplyLimit(BuildOrder::Early, 30*2);
	marineTank->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Marine, 6);
	marineTank->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Medic, 2);
	marineTank->add(BuildOrder::Early, BWAPI::UnitTypes::Terran_Firebat, 1);
	marineTank->add(BuildOrder::Early, BWAPI::UpgradeTypes::U_238_Shells);
	marineTank->add(BuildOrder::Early, BWAPI::TechTypes::Stim_Packs);
	marineTank->add(BuildOrder::Early, BWAPI::TechTypes::Scanner_Sweep);
	marineTank->setSupplyLimit(BuildOrder::Mid, 200*2);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Marine, 12);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Medic, 4);
	marineTank->add(BuildOrder::Mid, BWAPI::UnitTypes::Terran_Science_Vessel, 1);
	marineTank->add(BuildOrder::Mid, BWAPI::TechTypes::Tank_Siege_Mode);
	marineTank->add(BuildOrder::Mid, BWAPI::TechTypes::Irradiate);
	marineTank->add(BuildOrder::Mid, BWAPI::TechTypes::Stim_Packs);
	marineTank->add(BuildOrder::Mid, BWAPI::TechTypes::Optical_Flare);
	marineTank->add(BuildOrder::Mid, BWAPI::UpgradeTypes::U_238_Shells);
	marineTank->add(BuildOrder::Mid, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons);
	marineTank->add(BuildOrder::Mid, BWAPI::UpgradeTypes::Terran_Vehicle_Plating);

	//No end game units for marineTank
}

BuildOrder* LoadBuildOrder::getBuildOrder()
{
	return this->marineTank;
}

LoadBuildOrder::~LoadBuildOrder(void)
{
	delete this->marineTank;
}
