#include "../Includes.h"
#include "MurderMode.h"
#include "../Game/CGame.h"
#include "../Game/Human.h"
#include "../Utils/D3D11/Renderer.h"
#include "Config.h"

void MurderMode::Initialize()
{
	target = 0;
	gameWindow = NULL;
}

Object* MurderMode::GetTarget()
{
	return target;
}

void MurderMode::KillTarget()
{
	static auto head = -1;
	static auto chest = -1;
	static auto g = CGame::Get();
	static auto cfg = Config::Get()->GetGroup(xorstr_("misc"));
	static auto mm_bone = cfg->GetVariable(xorstr_("mm_bone"));
	auto world = g->GetWorld();
	if (!world || !target)
		return;

	auto human = EnfCast(target, Human);
	if (head < 0) {
		head = human->GetBoneIndexByName(xorstr_("head"));
		return;
	}

	if (chest < 0) {
		chest = human->GetBoneIndexByName(xorstr_("pelvis"));
		return;
	}
	

	auto pos = human->GetBonePositionWS(mm_bone->GetValue<int>() == 0 ? head : chest);

	auto bulletTable = *(Offset64**)(world + 0xD48);
	auto bulletTableSize = *(uint32_t*)(world + 0xD50);
	for (uint32_t i = 0; i < bulletTableSize; i++) {
		auto bullet2 = EnfCast(bulletTable[i], Object);
		bullet2->SetPosition(pos);
	}
}

void MurderMode::Update()
{
	if (gameWindow == NULL)
		gameWindow = FindWindowA(0, xorstr_("DayZ"));

	RECT rect;
	int width = 0;
	int height = 0;
	if (GetWindowRect(gameWindow, &rect))
	{
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	static auto g = CGame::Get();

	static auto cfg = Config::Get()->GetGroup(xorstr_("misc"));
	static auto mm_radius = cfg->GetVariable(xorstr_("mm_radius"));

	int radius = mm_radius->GetValue<int>();

	float curDist = radius;
	for (auto pl : players) {
		if (!pl)
			continue;

		float dist = EnfCast(pl, Human)->HeadDistance2D({ (float)width / 2, (float)height / 2, 0 });

		if (dist < curDist) {
			target = pl;
			curDist = dist;
		}
	}
	players.clear();

	if (target && EnfCast(target, Human)->HeadDistance2D({ (float)width / 2, (float)height / 2, 0 }) > radius) {
		target = NULL;
	}


	static auto mm_draw_radius = cfg->GetVariable(xorstr_("mm_draw_radius"));
	
	if(mm_draw_radius->GetValue<bool>())
		Renderer::Get()->Circle(width / 2, height / 2, radius, Color(255, 255, 255, 255));
}

void MurderMode::Add(Object* player)
{
	static auto g = CGame::Get();
	if (g->GetLocalPlayer() == player)
		return;

	players.push_back(player);
}
