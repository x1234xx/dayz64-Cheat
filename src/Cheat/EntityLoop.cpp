#include "../Includes.h"
#include "../Game/CGame.h"
#include "../Game/Object.h"
#include "../Game/Human.h"
#include "ServerCrasher.h"
#include "../Utils/D3D11/Renderer.h"
#include "EntityLoop.h"
#include "MurderMode.h"
#include "Config.h"
#include "..//Utils/Utils.h"
#include "../Game/HumanInventory.h"

void EntityLoop::Update()
{
	static auto om = OffsetManager::Get();
	static auto g = CGame::Get();
	static auto r = Renderer::Get();
	static auto mm = MurderMode::Get();
	static auto dh = Utils::Get();


	static auto cfg = Config::Get()->GetGroup(xorstr_("misc"));
	static auto cmm = cfg->GetVariable(xorstr_("murdermode"));

	auto objectArray = g->GetObjectArray();

	if (!objectArray)// || om->IsBadReadPtr(objectArray)) 
	{
		return;
	}

	auto localPlayer = g->GetLocalPlayer();
	auto localPosition = localPlayer->GetPosition();

	auto inventory = localPlayer->GetInventory();
	for (int i = 0; i < objectArray->GetSize(); i++)
	{
		Object* obj = objectArray->Get(i);
		if (obj)
		{
			if (cmm->GetValue<bool>()) {
				auto type = obj->GetType();

				if (type != NULL && type->isValid() && !strcmp(type->string, xorstr_("dayzplayer")) && g->IsPlayer(obj->GetNetworkId()))
					mm->Add(obj);
			}

			obj->Draw(localPlayer, localPosition);
		}
	}

	mm->Update();
	if (cmm->GetValue<bool>()) {
		mm->KillTarget();
	}

}