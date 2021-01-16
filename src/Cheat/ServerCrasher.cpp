#include "../Includes.h"
#include "../Utils/OffsetManager.h"
#include "../Game/CGame.h"
#include "../Game/Object.h"
#include "ServerCrasher.h"

#define ADDRESS 0x5E9B57;

void ServerCrasher::GetOriginal()
{
	const auto adr = GetAddress();
	this->originalByte = *(char*)adr;
}

const Offset64 ServerCrasher::GetAddress()
{
	return (Offset64)GetModuleHandle(NULL) + ADDRESS;
}

void ServerCrasher::Enable()
{
	const auto adr = GetAddress();
	const auto adr2 = (Offset64)GetModuleHandle(NULL) + 0x4BF9E0;

	DWORD old;
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr), 0x75, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, old, &old);

	VirtualProtect(reinterpret_cast<LPVOID>(adr2), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr2), 0x75, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr2), 1, old, &old);
}

void ServerCrasher::Disable()
{
	const auto adr = GetAddress();
	const auto adr2 = (Offset64)GetModuleHandle(NULL) + 0x4BF9E0;

	DWORD old;
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr), 0x74, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, old, &old);


	VirtualProtect(reinterpret_cast<LPVOID>(adr2), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr2), 0x74, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr2), 1, old, &old);
}

void ServerCrasher::Punch()
{
	static auto g = CGame::Get();
	typedef Offset64(__stdcall * fnPunch)(Object * source, Object * target, int, const char*, Vector3);
	static const auto punch = (fnPunch)((Offset64)GetModuleHandle(NULL) + 0x3AAAC0);

	//

	static const auto adr = (Offset64)GetModuleHandle(NULL) + 0x5E9B57;

	DWORD old;
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr), 0x75, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, old, &old);

	auto pl = g->GetLocalPlayer();
	Vector3 de = pl->GetPosition();
	punch(pl, pl, 0, xorstr_("FinisherHit"), de);
	
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr), 0x74, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, old, &old);
}

char ServerCrasher::GetOriginalByte()
{
	return originalByte;
}
