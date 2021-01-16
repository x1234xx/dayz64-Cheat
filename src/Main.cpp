#include "Includes.h"
#include "Cheat\PresentHook.h"
#include "Cheat\Config.h"
#include "Cheat\Menu.h"
#include "Utils\Utils.h"
#include "Game\Enfusion.h"
#include "Game\CGame.h"
#include "Game\Object.h"
#include "Game\Human.h"
#include "Game\World.h"
#include "Game\PlayerIdentity.h"
#include "Game\EntityAI.h"
#include "Game\HumanInventory.h"
#include "Game\HumanInputController.h"
#include "Game\Camera.h"
#include "Game\DayZPlayer.h"
#include "Cheat\Speedhack.h"
#include "Cheat\ServerCrasher.h"
#include "Cheat\MurderMode.h"
#include "Cheat\ItemHighlighter.h"
#include "Utils/D3D11/Renderer.h"

bool IsInProcess(const char* proc);
DWORD WINAPI Thread(void*);
DWORD WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpArgs)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (IsInProcess(xorstr_("DayZ_x64.exe"))) {

			/*if (OffsetManager::Get()->IsBadReadPtr((PVOID)0x100000)) {
				VirtualAlloc((PVOID)0x100000, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY);

				GetModuleFileNameW(hMod, (LPWSTR)0x100000, MAX_PATH);
				CreateThread(0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA(xorstr_("kernel32.dll")), xorstr_("LoadLibraryW")), (PVOID)0x100000, 0, 0);
				return FALSE;
			}
			VirtualFree((PVOID)0x100000, 0, MEM_RELEASE);*/
			//Thread(0);
			CreateThread(0, 0, Thread, 0, 0, 0);
		}
	}

	return TRUE;
}

bool IsInProcess(const char* proc)
{
	CHAR path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);

	return strcmp(PathFindFileNameA(path), proc) == 0;
}

DWORD WINAPI MainThread(void*)
{
	while (true)
	{
		static auto cfg = Config::Get()->GetGroup(xorstr_("misc"));
		static auto fc = cfg->GetVariable(xorstr_("freecam"));
		//static auto sc = cfg->GetVariable(xorstr_("servercrasher"));


#ifdef RISKY_FEATURES
		static auto sph = cfg->GetVariable(xorstr_("speedhack"));
		sph->GetValue<bool>() ? Speedhack::Get()->Enable() : Speedhack::Get()->Disable();
#endif 

		/*if (
		//sc->GetValue<bool>() && DHook::Get()->KeyDown(VK_F2))
		{
			ServerCrasher::Get()->Punch();
		}*/

		Camera::Get()->SetFreeCamActive(fc->GetValue<bool>());
		static auto g = CGame::Get();
		auto pl = g->GetLocalPlayer();
		if (pl) {
			auto contr = EnfCast(pl, Human)->GetInputController();
			if (contr) {
				contr->SetDisabled(fc->GetValue<bool>());
			}
		}

		Sleep(100);
	}
}


void CallScript()
{
//static const Offset64 FN_ReleaseScript = (Offset64)Base + 0x297950;

//	



//	static Offset64 previousModule = NULL;

	//while (previousModule)
	//	previousModule = ReleaseScript(previousModule);

	static EnScript* script = nullptr;
	if (script)
	{
		//script->Exit();
		script = nullptr;
		CallScript();
	}
	else {
		CHAR path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		PathRemoveFileSpecA(path);

		std::string script_path(path);
		script_path += "\\";
		script_path += xorstr_("script.c");
		script = EnScript::LoadScript(script_path);
		Beep(1000, 500);
	}
	
	
}

DWORD WINAPI UpdateThread(void*)	
{
	while (true) 
	{
		static auto util = Utils::Get();
	//	if (mtx.try_lock()) {
		//	game->UpdateObjects();
	//		mtx.unlock();
		//}

		if (util->IsKeyDown(VK_DELETE)) {
			CallScript();
			Sleep(2000);
			/*typedef Offset64(__fastcall* fnMatrixShit)(Object* entity, float* matrix12, char a3);
			static const auto base = (Offset64)GetModuleHandle(NULL);
			static auto test = (fnMatrixShit)(base + 0x51D450);
			float matrix[12] = { 
				0.0f, 3.0f, 0.0f,
				1.0f, 2.5f, 1.0f,
				0.0f, 0.0f, 100.f,
				30.f, 20.f, 10.f
			};

			static bool call = false;
			call = call;
			test(game->GetLocalPlayer(), matrix, call);*/
		}

		Sleep(1);
	}
}

DWORD WINAPI Thread(void*)
{
	Beep(50, 500);

	CGame::Get()->Initialize();
	World::Get()->Initialize();
	Camera::Get()->Initialize();
	MurderMode::Get()->Initialize();

	HumanInventory::InitClass();
	EntityAI::InitClass();
	Object::InitClass();
	Human::InitClass();
	HumanInputController::InitClass();
	PlayerIdentity::InitClass();
	DayZPlayer::InitClass();
	EnScript::Initialize();

	Sleep(500);

	Config::Get()->Initialize();

	Sleep(250);

	Menu::Get()->Initialize();

	Beep(750, 500);

	auto d = Utils::Get();
	while (!d->Initialize())
		Sleep(500);

	//ItemHighlighter::Get()->Init();

	Sleep(500);

	Beep(1000, 500);
	Hooks::oPresent = d->HookPresent(Hooks::hkPresent);
	

//	0x7ffa062ec820;
	Sleep(5000);
	CreateThread(0, 0, MainThread, 0, 0, 0);

	CreateThread(0, 0, UpdateThread, 0, 0, 0);

	return 0;
}
/*
extern "C" __declspec(dllexport) LRESULT WINAPI derp(int code, WPARAM wParam, LPARAM lParam) {
	return(CallNextHookEx(NULL, code, wParam, lParam));
}
*/
