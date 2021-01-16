#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class PlayerIdentityShared : public EnfClassSingleton<PlayerIdentityShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};

class PlayerIdentity
{
protected:
	static PlayerIdentityShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("PlayerIdentity")); };

	const char* GetName();
	const int GetNetworkId();
};