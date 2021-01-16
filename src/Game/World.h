#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class Object;

class World : public EnfClassSingleton<World>
{
public:
	bool Initialize(const char* className = xorstr_("World"));
	Offset64 GetScriptModule();
};