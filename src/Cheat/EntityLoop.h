#pragma once

#include "../Singleton.h"
#include "../Game/Enfusion.h"
#include "../Game/Object.h"

class EntityLoop : public Singleton<EntityLoop>
{
public:
	void Update();
};


