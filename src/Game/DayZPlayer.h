#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class Object;
class Renderer;

class DayZPlayerShared : public EnfClassSingleton<DayZPlayerShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};

class DayZPlayer
{
protected:
	static DayZPlayerShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("DayZPlayer")); };
	void PunchTarget(Object* target);
};