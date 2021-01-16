#include "..//Includes.h"
#include "EntityAI.h"
#include "HumanInventory.h"

HumanInventory* EntityAI::GetInventory()
{
	typedef HumanInventory*(__fastcall * fnGetInventory)(EnfPtr object);
	static const fnGetInventory getInventory = (fnGetInventory)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetInventory"));
	return getInventory(this);
}
