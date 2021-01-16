#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class HumanInputControllerShared : public EnfClassSingleton<HumanInputControllerShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};


class HumanInputController
{
protected:
	static HumanInputControllerShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("HumanInputController")); };
	void SetDisabled(bool state);
};
