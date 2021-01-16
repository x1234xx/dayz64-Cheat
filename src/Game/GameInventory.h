#pragma once

#include "Enfusion.h"

class GameInventory : public EnfClassSingleton<GameInventory>
{
public:
	bool Initialize(const char* className = xorstr_("GameInventory"));
};