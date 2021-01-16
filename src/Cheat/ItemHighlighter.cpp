#include "../Includes.h"
#include "ItemHighlighter.h"
#include "../Game/Enfusion.h"
#include "../Utils/D3D11/Renderer.h"
#include "../Utils/Utils.h"

/*
LRESULT STDMETHODCALLTYPE hkWndProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	static auto hl = ItemHighlighter::Get();


	switch (msg) 
	{
	case WM_CHAR:
		switch (wParam) {
			case 0x08:
			case 0x0A:
			case 0x1B:
			case 0x09:
			case 0x0D:
				break;
			default:
				if (hl->IsRecording())
					hl->AddBuffer((wchar_t)wParam);
				break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			if (hl->IsRecording()) {
				hl->AddBufferAsItem();
				hl->SetRecording(false);
			}
			break;
		}
	}

	if (hl->IsRecording())
		return 0;

	return CallWindowProc(hl->GetWndProc(), hWnd, msg, wParam, lParam);
}

void ItemHighlighter::Init()
{
	recording = false;
	CleanBuffer();

	//wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(FindWindowA(0, xorstr_("DayZ")), GWLP_WNDPROC, (LONG_PTR)hkWndProc));
}

void ItemHighlighter::AddItem(std::wstring item)
{
	if(item.length() > 0)
		ihl_strings.push_back(item);
}

bool ItemHighlighter::ShouldHighlight(EString* name)
{
	for (std::wstring s : ihl_strings)
	{
		if (name->contains(s))
			return true;
	}

	return false;
}

void ItemHighlighter::Draw()
{
	static auto r = Renderer::Get();
	static auto d = Utils::Get();

	if (recording)
	{
		r->Text(500, 10, "Recording input (press enter to finish)", Color(255, 255, 255, 255));
		r->Text(500, 30, buffer, Color(255, 255, 255, 255));
	}
	else {
		r->Text(500, 10, "Press F9 to add an item!", Color(255, 255, 255, 255));
		if (d->IsKeyDown(VK_F9))
			recording = true;
		
		int i = 0;
		for (auto w : ihl_strings) {
			i++;
			r->Text(500, 10 + 20 * i, w, Color(255, 255, 255, 255));
		}
	}
}

bool ItemHighlighter::IsRecording()
{
	return recording;
}

void ItemHighlighter::SetRecording(bool recording)
{
	this->recording = recording;
}

WNDPROC ItemHighlighter::GetWndProc()
{
	return wndProc;
}

std::wstring& ItemHighlighter::GetBuffer()
{
	return buffer;
}

void ItemHighlighter::AddBuffer(std::wstring buf)
{
	buffer += buf;
}

void ItemHighlighter::AddBuffer(wchar_t c)
{
	buffer += c;
}

void ItemHighlighter::AddBufferAsItem()
{
	this->AddItem(buffer);
	this->CleanBuffer();
}

void ItemHighlighter::CleanBuffer()
{
	buffer = L"";
}
*/