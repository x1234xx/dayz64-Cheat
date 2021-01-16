#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

class Renderer;
class HumanInventory;

class ObjectShared : public EnfClassSingleton<ObjectShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};

class Object
{
protected:
	static ObjectShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("Object")); };
	Vector3& GetPosition();
	void SetPosition(Vector3 pos);
	EString* GetType();
	EString* GetCleanName();
	EString* GetModelPath();
	EString* GetConfigName();
	void Draw(Object* localPlayer, Vector3& localPosition);
	float DistanceFrom(Vector3& from);
	float DistanceFrom2D(Vector3& from2d);
	void TestDamage(Object* target);
	HumanInventory* GetInventory();
	Object* GetEntityInHands();
	Offset64* GetConfigVariable(const char* entryName);

	bool IsKindaBarrel();

	int GetNetworkId();
	bool IsDead();
private:
	Offset64 GetTypeClass();
};