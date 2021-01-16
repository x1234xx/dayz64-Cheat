#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class Camera : public EnfClassSingleton<Camera>
{
public:
	bool Initialize(const char* className = xorstr_("Camera"));
	void SetFreeCamActive(bool active);
private:
	void* GetFreeDebugCamera();
};