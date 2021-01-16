#include "../Includes.h"
#include "CGame.h"
#include "World.h"


bool World::Initialize(const char* className)
{
	return EnfClass::Initialize(className);
}

Offset64 World::GetScriptModule()
{
	Offset64 CWorldClass = enfClass->GetClassPtr();
	if (CWorldClass)
		return *(Offset64*)(CWorldClass + 0x18);

	return NULL;
}