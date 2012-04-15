#pragma once
#include <vector>

class Utils
{
public:
	Utils(void);
	~Utils(void);

	template <typename T>
	static void vectorRemoveElement(std::vector<T*>* v, T* e)
	{
		v->erase(remove(v->begin(), v->end(), e));
	}
};
