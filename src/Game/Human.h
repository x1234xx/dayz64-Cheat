#pragma once

#include "../Utils/OffsetManager.h"
#include "Enfusion.h"

#define bone_head xorstr_("head")
#define bone_pelvis xorstr_("pelvis")
#define bone_neck xorstr_("neck")
#define bone_rightarm xorstr_("rightarm")
#define bone_rightforearm xorstr_("rightforearm")
#define bone_righthand xorstr_("righthand")
#define bone_rightupleg xorstr_("rightupleg")
#define bone_rightleg xorstr_("rightleg")
#define bone_rightfoot xorstr_("rightfoot")
#define bone_leftarm xorstr_("leftarm")
#define bone_leftforearm xorstr_("leftforearm")
#define bone_lefthand xorstr_("lefthand")
#define bone_leftupleg xorstr_("leftupleg")
#define bone_leftleg xorstr_("leftleg")
#define bone_leftfoot xorstr_("leftfoot")

class Object;
class Renderer;
class HumanInputController;

class HumanShared : public EnfClassSingleton<HumanShared>
{
public:
	bool Initialize(const char* className) { return EnfClass::Initialize(className); }
};

class Human
{
protected:
	static HumanShared* enfClass;
public:
	static void InitClass() { enfClass->Get()->Initialize(xorstr_("Human")); };
	uint32_t GetBoneIndexByName(const char* boneName);
	Vector3& GetBonePositionWS(uint32_t boneIndex);
	HumanInputController* GetInputController();
	void Draw(Renderer* r, Color color);
	void Test(Object* target);
	void UnlinkFromLocalSpace();
	float HeadDistance2D(Vector3 from);
private:
	void DrawBoneLine(Renderer* r, Color color, uint32_t from, uint32_t to);
};