#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class Object;
class PlayerIdentity;

//extern std::mutex mtx;

class CGame : public EnfClassSingleton<CGame>
{
public:
	bool Initialize(const char* className = xorstr_("CGame"));
	Object* GetLocalPlayer();
	void* GetGameInstance();
	Offset64 GetWorld();


	Vector3& GetScreenPos(Vector3& worldPosition);

	void SpoofIsServer(bool spoof);

	bool IsServer();

	char* ObjectGetDisplayName(Object* entity);

	Offset64 GetScriptModule();

private:
	AutoArray<Object*>* objectArray;
	AutoArray<PlayerIdentity*>* identityArray;
	void UpdateObjects();
	void StorePlayerIdentities();
public:
	const char* GetPlayerName(int player_id);
	PlayerIdentity* GetPlayerIdentity(int player_id);
	void SelectPlayer(Object* player);
	bool IsPlayer(int player_id);
	Object* GetClosetPlayer();

	AutoArray<Object*>* GetObjectArray();
	void NewFrame();
	void PostFrame();
	bool isVisible(Object* target);
};