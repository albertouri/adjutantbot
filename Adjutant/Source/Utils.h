#pragma once
#include <algorithm>
#include "BWAPI.h"
#include <vector>

class Utils
{
public:
	Utils(void);
	~Utils(void);

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
};
