#include "PresentHook.h"
#include "../Utils/D3D11/Renderer.h"
#include "../Game/CGame.h"
#include "../Game/Object.h"
#include "../Game/Human.h"
#include "../Game/World.h"
#include "../Game/Camera.h"
#include "../Game/DayZPlayer.h"
#include "EntityLoop.h"
#include "Speedhack.h"
#include "ServerCrasher.h"
#include "Menu.h"
#include "Config.h"
#include "ItemHighlighter.h"

long __stdcall Hooks::hkPresent(IDXGISwapChain* pSwapChain, unsigned int syncIntreval, unsigned int flags)
{
	static auto r = Renderer::Get();
	static bool bOnce = false;
	static auto el = EntityLoop::Get();
	static bool bHideMenu = false;
//	static auto ihl = ItemHighlighter::Get();
	static auto cfg = Config::Get()->GetGroup(xorstr_("misc"));
	static auto print_pos = cfg->GetVariable(xorstr_("print_position"));

	if (!bOnce)
	{
	//	r->Initialize(pSwapChain);
	//	r->SetFPS(30);
	//	auto f = r->CreateFont(L"Arial", 12.f, DFont::STYLE_NORMAL, true);
	//	r->SetFont(f);
		r->Init(pSwapChain);
		bOnce = true;
	}

	if (r->Begin(30)) {

		//r->Text(500, 50, "HELLO FROM THE OTHER SIDE", Color(255, 00, 255, 255));


		static auto g = CGame::Get();

		Object* p = g->GetLocalPlayer();

		if (p)
		{
			g->NewFrame();

			el->Update();

			if (print_pos->GetValue<bool>())
			{
				auto position = p->GetPosition();
				auto str = r->PrepareString(xorstr_(L"Position (XYZ) { %i, %i, %i }"), (int)position.x, (int)position.y, (int)position.z);

				r->SetDrawColor(Color(137, 40, 133, 255));
				r->AddText(&font_default, 200, 0, str, FONT_ALIGN_LEFT);
			}

			g->PostFrame();
		}

		if (Utils::Get()->IsKeyDown(VK_INSERT)) {
			bHideMenu = !bHideMenu;
		}

		if (!bHideMenu) //&& !ihl->IsRecording())
			Menu::Get()->Draw();

		//r->SetDrawColor(Color(255, 255, 255, 255));
		//r->AddFilledLine(DirectX::XMFLOAT4(10, 50, 400, 60));
		//r->Circle(50, 400, 60, Color(255, 255, 255, 255));
		//	ihl->Draw();

		r->End();

	}
	r->Present(pSwapChain);

	return oPresent(pSwapChain, syncIntreval, flags);
}