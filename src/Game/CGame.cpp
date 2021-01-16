#include "../Includes.h"
#include "Object.h"
#include "CGame.h"
#include "PlayerIdentity.h"
#include "../Utils/Utils.h"
#include "../Utils/D3D11/Renderer.h"
#include <ctime>

//std::mutex mtx;

bool CGame::Initialize(const char* className)
{
	identityArray = 0;
	objectArray = 0;
	EnfClass::Initialize(className);
	initialized = true;
	return true;
}

Object* CGame::GetLocalPlayer()
{
	if (!IsInitialized())
		return NULL;

	typedef Offset64(__stdcall* fnGetPlayer)();
	static const fnGetPlayer getPlayer = (fnGetPlayer)enfClass->GetFunctionByName(xorstr_("GetPlayer"));

	if (!getPlayer)
		return NULL;


	//Object* ent = (Object*)getPlayer();

	//ent->Initialize(OffsetManager::Get()->GetObjectClass());

	return (Object*)getPlayer();
}

#define SIGNATURE_GETGAME xorstr_("4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 89 08")

void* CGame::GetGameInstance()
{
	typedef void*(*fnGetGame)();
	static fnGetGame getGameFn = 0;
	if (!getGameFn)
	{
		auto ret = OffsetManager::Get()->PatternScan(SIGNATURE_GETGAME);
		Offset64 func_ptr = *(Offset64*)(ret + 3);
		Offset64 func = (Offset64)ret + (Offset32)func_ptr + 7;

		getGameFn = reinterpret_cast<fnGetGame>(func - 0x100000000);

		//printf("getGameFn = %llx\n", getGameFn);

	}

	if(!getGameFn)
		return 0;
	
	return getGameFn();
}

Offset64 CGame::GetWorld()
{
	if (!IsInitialized())
		return NULL;

	typedef Offset64(__stdcall* fnGetWorld)();
	static const fnGetWorld getWorld = (fnGetWorld)enfClass->GetFunctionByName(xorstr_("GetWorld"));

	if (!getWorld)
		return NULL;

	Offset64 world = getWorld();

	//ent->Initialize(OffsetManager::Get()->GetObjectClass());

	return world;
}

Vector3& CGame::GetScreenPos(Vector3& worldPosition)
{
	Vector3 screenPosition(0, 0, 0);

	typedef Vector3&(__fastcall* fnGetScreenPos)(void* null, Vector3& screen, Vector3& world);
	static const fnGetScreenPos getScreenPos = (fnGetScreenPos)enfClass->GetFunctionByName(xorstr_("GetScreenPos"));

	if (!getScreenPos)
	{
		Beep(250, 500);
		return screenPosition;
	}

	return getScreenPos(NULL, screenPosition, worldPosition);
}

void CGame::SpoofIsServer(bool spoof)
{
	/*if (!IsInitialized())
		return;

	static const auto isServer = (Offset64)enfClass->GetFunctionByName(xorstr_("IsServer"));

	Offset64 func_ptr = *(Offset64*)(isServer + 17);
	Offset64 func = (Offset64)isServer + (Offset32)func_ptr + 7;
	*/
	const Offset64 adr = ((Offset64)GetModuleHandle(NULL) + 0x5C53F3);

	DWORD old;
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<LPVOID>(adr), spoof ? 0x50 : 0x48, 1);
	VirtualProtect(reinterpret_cast<LPVOID>(adr), 1, old, &old);
}

bool CGame::IsServer()
{
	if (!IsInitialized())
		return false;

	typedef Offset64(__stdcall * fnIsServer)(Offset64);
	static const auto isServer = (fnIsServer)((Offset64)GetModuleHandle(NULL) + 0x5C53F0);

	static const auto networkManager = (Offset64)GetModuleHandle(NULL) + 0xD99AB0;
	return isServer(networkManager);
}

class ARG0
{
public:
	Object* Argument;
};

class ARG1
{
public:
	char Argument[32];
};

class ArgumentList
{
public:
	ARG0* in;
	ARG1* out;
};

char* CGame::ObjectGetDisplayName(Object* object)
{
	typedef char(__stdcall* fnObjectGetDisplayName)(void*, ArgumentList** argArray);
	static const fnObjectGetDisplayName objectGetDisplayName = (fnObjectGetDisplayName)enfClass->GetFunctionByName("ObjectGetDisplayName");

	if (!objectGetDisplayName)
		return NULL;
	
	ArgumentList* args = new ArgumentList;
	
	ARG0 arg0;
	arg0.Argument = object;
	
	ARG1 arg1;

	args->in = &arg0;
	args->out = &arg1;

	objectGetDisplayName(NULL, &args);

	MessageBoxA(NULL, arg1.Argument, 0, 0);

	return NULL;
}

Offset64 CGame::GetScriptModule()
{
	Offset64 CGameClass = enfClass->GetClassPtr();
	if (CGameClass)
		return *(Offset64*)(CGameClass + 0x18);

	return NULL;
}

void CGame::UpdateObjects()
{
	if (!IsInitialized())
		return;

	auto localPlayer = GetLocalPlayer();

	if (!localPlayer)
	{
		//printf("no localplayer...\n");
		return;
	}

	typedef void(__stdcall* tFunction)(void*, const Vector3&, float, AutoArray<Object*>*, void*);
	static const tFunction function = (tFunction)enfClass->GetFunctionByName(xorstr_("GetObjectsAtPosition"));


	//mtx.lock();
	if (!objectArray) {
	//	FreeArray(objectArray);
		objectArray = AllocateArray<Object*>();
	}
	
	auto pos = localPlayer->GetPosition();
	function(nullptr, pos, 1000.0f, objectArray, nullptr);


	//mtx.unlock();

	//static auto dq = DrawingQueue::Get();
	//dq->QueueArray<Offset64>(objectArray);
		
}

void CGame::StorePlayerIdentities()
{
	typedef void(__stdcall* tFunction)(void*, AutoArray<PlayerIdentity*>*);
	static const tFunction function = (tFunction)enfClass->GetFunctionByName(xorstr_("GetPlayerIndentities"));

	if (!identityArray) {
		//	FreeArray(objectArray);
		identityArray = AllocateArray<PlayerIdentity*>();
	}
	function(nullptr, identityArray);

	//identityArray->MergeWithVector(identityVector);

	//FreeArray(identityArray);
}

const char* CGame::GetPlayerName(int player_id)
{
	if (!identityArray)
		return "fat error";

	for (int i = 0; i < identityArray->GetSize(); i++)
	{
		auto pi = identityArray->Get(i);
		if (pi && pi->GetNetworkId() == player_id)
			return pi->GetName();
	}

	return "error";
}

PlayerIdentity* CGame::GetPlayerIdentity(int player_id)
{
	if (!identityArray)
		return NULL;

	for (int i = 0; i < identityArray->GetSize(); i++)
	{
		auto pi = identityArray->Get(i);
		if (pi && pi->GetNetworkId() == player_id)
			return pi;
	}

	return NULL;
}

void CGame::SelectPlayer(Object* player)
{
	if (!player)
		return;

	auto id = GetPlayerIdentity(player->GetNetworkId());

	if (!id)
		return;

	typedef void(__stdcall* tFunction)(void*, void*, void*);
	static const tFunction function = (tFunction)enfClass->GetFunctionByName(xorstr_("SelectPlayer"));
	function(player, id, player);
}

bool CGame::IsPlayer(int player_id)
{
	if (!identityArray)
		return false;

	for (int i = 0; i < identityArray->GetSize(); i++)
	{
		auto pi = identityArray->Get(i);
		if (pi && pi->GetNetworkId() == player_id)
			return true;
	}

	return false;
}

Object* CGame::GetClosetPlayer()
{;
	Object* local = this->GetLocalPlayer();

	if (!local)
		return NULL;

	Object* target = NULL;
	float dstClosest = 1000.f;
	for (int i = 0; i < objectArray->GetSize(); i++)
	{
		Object* obj = (Object*)objectArray->Get(i);
		if (!obj)
			continue;

		if (obj == local)
			continue;
		
		auto type = obj->GetType();
		if (!type)
			continue;
		
		if (!strcmp(type->string, xorstr_("dayzplayer")))
		{
			if (local->DistanceFrom(obj->GetPosition()) < dstClosest)
				target = obj;
		}
	}

	return target;
}

AutoArray<Object*>* CGame::GetObjectArray()
{
	return objectArray;
}

void CGame::NewFrame()
{
	//DrawingQueue::Get()->Clear();

	//UpdateObjects();
	//	identityVector.clear();
//	
	static int c = 150;
	if (c > 45) {
		UpdateObjects();
		StorePlayerIdentities();
		c = 0;
	}
	c++;
}

void CGame::PostFrame()
{
	//FreeArray(identityArray);
	//identityVector.clear();
}

bool CGame::isVisible(Object* target)
{
	return false;
}
