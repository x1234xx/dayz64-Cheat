#include "../Includes.h"
#include "Utils.h" 

// AS OF 2019-04-14
// 48 8B 05 ? ? ? ? 48 8B CA 48 8D 15 ? ? ? ? 
/*#define SIGNATURE_GAME_RENDERER "48 8B 05 ? ? ? ? 48 8B CA 48 8D 15 ? ? ? ?"

void DX11Renderer::Initialize()
{
	auto om = OffsetManager::Get();

	// Get renderer
	auto ret = om->PatternScan(SIGNATURE_GAME_RENDERER);
	Offset64 class_ptr = *(Offset64*)(ret + 3);
	Offset64 _class = (Offset64)ret + (Offset32)class_ptr + 7;
	this->IRenderer = *(Offset64*)_class;


}*/

/*
#define SIGNATURE_DISCORD_CREATEHOOK		xorstr_("40 53 55 56 57 41 54 41 56 41 57 48 83 EC 60")
#define SIGNATURE_DISCORD_ENABLEHOOK		xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 20 33 F6 8B FA")
#define SIGNATURE_DISCORD_ENABLEHOOKQUE		xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 57")
#define SIGNATURE_DISCORD_GETASYNCKEYSTATE	xorstr_("40 53 48 83 EC 20 8B D9 FF 15 ? ? ? ?")
#define SIGNATURE_DISCORD_PRESENT			xorstr_("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B D9 41 8B F8")
#define SIGNATURE_DISCORD_INTERCEPT			xorstr_("FF 15 ? ? ? ? 8B D8 48 8D 4C 24 ? E8 ? ? ? ? 48 8B 74 24 ?")
*/

LRESULT STDMETHODCALLTYPE hkWndProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	static auto u = Utils::Get();

	switch (msg)
	{
	case WM_KEYDOWN:
		u->SetKeyState((int)wParam, true);
		break;
	case WM_KEYUP:
		u->SetKeyState((int)wParam, false);
		break;
	}

	return CallWindowProc(u->GetWndProc(), hWnd, msg, wParam, lParam);
}

bool Utils::Initialize()
{
	if(!GetModuleHandleA(xorstr_("dxgi.dll")))
		return false;

	auto om = OffsetManager::Get();
	wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(FindWindowA(0, xorstr_("DayZ")), GWLP_WNDPROC, (LONG_PTR)hkWndProc));

	auto materialSystem = *(Offset64*)((Offset64)(GetModuleHandle(NULL)) + 0xDCB960);
	auto renderer = *(Offset64*)(materialSystem + 0x148);
	auto wrappedSwapChain = *(Offset64*)(renderer + 0xE8);

	pVT = **(Offset64**)(wrappedSwapChain + 0xB0);

	return true;

	/*auto discord = GetModuleHandle(L"DiscordHook64.dll");

	//printf("DiscordHook64: %llx\n\n", discord);
	if (!discord)
		return false;

	DCreateHook			= reinterpret_cast<fnCreateHook>(om->PatternScan(SIGNATURE_DISCORD_CREATEHOOK, 0, discord));
	DEnableHook			= reinterpret_cast<fnEnableHook>(om->PatternScan(SIGNATURE_DISCORD_ENABLEHOOK, 0, discord));
	DEnableHookQue		= reinterpret_cast<fnEnableHookQue>(om->PatternScan(SIGNATURE_DISCORD_ENABLEHOOKQUE, 0, discord));
	DGetAsyncKeyState	= reinterpret_cast<fnGetAsyncKeystate>(om->PatternScan(SIGNATURE_DISCORD_GETASYNCKEYSTATE, 0, discord));
	DPresent			= reinterpret_cast<PVOID>(om->PatternScan(SIGNATURE_DISCORD_INTERCEPT, 0, discord)); //reinterpret_cast<PVOID>((Offset64)discord + 0x2DC338); //(om->PatternScan(SIGNATURE_DISCORD_PRESENT, 0, discord));


	DPresent = (PVOID)((Offset64)DPresent + *(Offset32*)((Offset64)DPresent + 2) + 6);


	//printf("DCreateHook: %llx\n\n", DCreateHook);
	//printf("DEnableHook: %llx\n\n", DEnableHook);
	//printf("DEnableHookQue: %llx\n\n", DEnableHookQue);
	//printf("DGetAsyncKeyState: %llx\n\n", DGetAsyncKeyState); 
	//printf("DPresent: %llx\n\n", DPresent);

	return (DCreateHook && DEnableHook && DEnableHookQue && DGetAsyncKeyState && DPresent);*/
}

fnPresent Utils::HookPresent(fnPresent fnHook)
{
	auto pTarg = pVT + 0x40;

	auto fnOriginal = *(Offset64*)(pTarg);

	DWORD old;
	VirtualProtect((PVOID)pTarg, 8, PAGE_READWRITE, &old);
	*(Offset64*)(pTarg) = (Offset64)fnHook;
	VirtualProtect((PVOID)pTarg, 8, old, &old);
	
	return (fnPresent)fnOriginal;
}

void Utils::UnhookPresent()
{
	
}

WNDPROC Utils::GetWndProc()
{
	return wndProc;
}

void Utils::SetKeyState(int key, bool down)
{
	keyStates[key] = down;
}

bool Utils::IsKeyDown(int vKey)
{
	if (keyStates[vKey])
	{
		SetKeyState(vKey, false);
		return true;
	}

	return false;
}
