#pragma once

#include "../Singleton.h"

/*
class ItemHighlighter : public Singleton<ItemHighlighter>
{
private:
	std::vector<std::wstring> ihl_strings;
	bool recording;
	WNDPROC wndProc;
	std::wstring buffer;
public:
	void Init();
	void AddItem(std::wstring item);
	bool ShouldHighlight(class EString* name);
	void Draw();
	bool IsRecording();
	void SetRecording(bool recording);
	WNDPROC GetWndProc();
	std::wstring& GetBuffer();
	void AddBuffer(std::wstring buf);
	void AddBuffer(wchar_t c);
	void AddBufferAsItem();
	void CleanBuffer();
};

LRESULT __stdcall hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
*/