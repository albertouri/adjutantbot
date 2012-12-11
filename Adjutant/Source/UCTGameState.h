#pragma once
#include <algorithm>
#include "UCTAttackAction.h"
#include "UCTJoinAction.h"
#include "UCTGroup.h"

class UCTGameState
{
public:
	static float** matrix;

	UCTGameState(void);
	~UCTGameState(void);
	void initTrainingMatrix();
	bool isLeaf();
	bool isValidAction(UCTAction* action);
	bool willTriggerSimulation(UCTAction* action);
	float getRewardValue();
	void markGroupsForAction(UCTAction* action);
	void simulate();
	
	std::vector<UCTGroup*> myGroups;
	std::vector<UCTGroup*> enemyGroups;
	std::map<UCTGroup*, UCTAction*> groupActionMap;
private:
	static const int SIM_FRAMES_PER_STEP = 50;
	static const int SIM_JOIN_DISTANCE = 50;
	static const int SIM_ATTACK_DISTANCE = 100;
	static float SIM_ATTACK_ADVANTAGE_LOW_LIMIT;
	static float SIM_ATTACK_ADVANTAGE_HIGH_LIMIT;

	UCTGroup* getClosestGroup(UCTGroup* group, std::vector<UCTGroup*>* otherGroups);
	void simulateAttacks(std::map<UCTGroup*, std::set<UCTGroup*>>* attackingGroups);
	void simulateJoins(std::map<UCTJoinAction*, std::set<UCTGroup*>>* joiningGroups);
};
