#pragma once

#include "Enfusion.h"

class Object;

class HumanInventoryShared : public EnfClassSingleton<HumanInventoryShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};

class HumanInventory
{
protected:
	static HumanInventoryShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("HumanInventory")); };
	Object* GetEntityInHands();
	bool TestAddEntityInHands(Object* entity);
};