#pragma once

#include "Enfusion.h"

class HumanInventory;

class EntityAIShared : public EnfClassSingleton<EntityAIShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};

class EntityAI
{
protected:
	static EntityAIShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("EntityAI")); };
	HumanInventory* GetInventory();
};