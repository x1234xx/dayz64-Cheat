#include "PlayerIdentity.h"

const char* PlayerIdentity::GetName()
{
	typedef char*(__fastcall* fnGetFullName)(Offset64 player_identity);
	static const fnGetFullName getFullName = (fnGetFullName)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetName"));

	if (!getFullName)
		return "error";

	return getFullName(reinterpret_cast<Offset64>(this));
}

const int PlayerIdentity::GetNetworkId()
{
	return *(int*)((Offset64)this + 0x30);
}
