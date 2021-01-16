#include "../Includes.h"
#include "Camera.h"

#define SIGNATURE_FREE_DEBUG_CAMERA xorstr_("48 83 EC 38 48 8B 05 ?? ?? ?? ?? 48 85 C0")

bool Camera::Initialize(const char* className)
{
	return EnfClass::Initialize(className);
}

void Camera::SetFreeCamActive(bool active)
{
	typedef __int64(__fastcall* tFunction)(void*, bool);
	static const tFunction function = (tFunction)enfClass->GetFunctionByName(xorstr_("SetActive"));
	auto c = GetFreeDebugCamera();
	if (!c)
		return;

	function(c, active);
}

void* Camera::GetFreeDebugCamera()
{

	typedef void*(__fastcall * tFunction)();
	static const tFunction function = (tFunction)OffsetManager::Get()->PatternScan(SIGNATURE_FREE_DEBUG_CAMERA);//((Offset64)GetModuleHandle(NULL) + 0x409A80);
	return function();
}
