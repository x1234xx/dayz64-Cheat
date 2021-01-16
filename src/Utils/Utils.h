#pragma once

#include "../Includes.h"
#include "../Singleton.h"
#include "../Utils/OffsetManager.h"


/*
*******************************************************
*Discord Hook******************************************
*Credits:**********************************************
*https://github.com/GayPig/DirectX11-hook-with-discord*
*******************************************************
*/

typedef long(__stdcall *fnPresent) (IDXGISwapChain* pSwapChain, UINT syncIntreval, UINT flags);
/*
typedef int(__stdcall* fnCreateHook)(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
typedef int(__stdcall* fnEnableHook)(LPVOID pTarget, BOOL enable);
typedef int(__stdcall* fnEnableHookQue)(VOID);
typedef SHORT(__stdcall* fnGetAsyncKeystate)(int vKey);*/

class Utils : public Singleton<Utils>
{
public:
	bool Initialize();

//	bool KeyPressed(int vKey);
	bool IsKeyDown(int vKey);

	fnPresent HookPresent(fnPresent fnHook);
	void UnhookPresent();
	WNDPROC GetWndProc();
	
	void SetKeyState(int key, bool down);
private:
	WNDPROC wndProc;
	bool keyStates[0xFF];
	Offset64 pVT;
};