#include "../Includes.h"
#include "HumanInventory.h"

Object* HumanInventory::GetEntityInHands()
{
	typedef Object*(__fastcall * fnGetEntityInHands)(EnfPtr inventory);
	static const fnGetEntityInHands getEntityInHands = (fnGetEntityInHands)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetEntityInHands"));
	return getEntityInHands(this);
}

bool HumanInventory::TestAddEntityInHands(Object* entity)
{
	typedef Object* (__fastcall* fnGetEntityInHands)(EnfPtr inventory, Object* entity, bool a3, bool a4, int a5, bool a6, bool a7);
	static const fnGetEntityInHands testAddEntityInHands = (fnGetEntityInHands)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("TestAddEntityInHands"));
	return testAddEntityInHands(this, entity, false, false, 0, false, false);
}
