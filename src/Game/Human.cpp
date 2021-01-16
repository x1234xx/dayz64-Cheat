#include "../Includes.h"
#include "Human.h"
#include "CGame.h"
#include "../Utils/D3D11/Renderer.h"
#include "../Cheat/Config.h"
#include "../Cheat/ServerCrasher.h"

uint32_t Human::GetBoneIndexByName(const char* boneName)
{
	typedef uint32_t(__fastcall * fnGetBoneIndexByName)(void*, const char*);
	static const fnGetBoneIndexByName getBoneIndexByName = (fnGetBoneIndexByName)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetBoneIndexByName"));

	if (!getBoneIndexByName)
		return 0;

	return getBoneIndexByName(this, boneName);
}

Vector3& Human::GetBonePositionWS(uint32_t boneIndex)
{
	typedef void*(__fastcall * fnGetBonePositionWS)(void*, Vector3*, uint32_t);
	static const fnGetBonePositionWS getBonePositionWS = (fnGetBonePositionWS)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetBonePositionWS"));

	Vector3 out(0, 0, 0);
	if (!getBonePositionWS)
		return out;
		
	getBonePositionWS(this, &out, boneIndex);
	return out;
}

HumanInputController* Human::GetInputController()
{
	typedef HumanInputController*(__fastcall * fnGetInputController)(void*);
	static const fnGetInputController getInputController = (fnGetInputController)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetInputController"));

	if (!getInputController)
		return NULL;

	return getInputController(this);
}

#define get_bone(bonename) GetBoneIndexByName(bonename)
#define bone(name) static auto name = get_bone(bone_##name); if(name < 0) { name = get_bone(bone_##name); return; }

void Human::Draw(Renderer* r, Color color)
{
	bone(head);
	bone(neck);
	bone(pelvis);
	bone(leftupleg);
	bone(leftleg);
	bone(leftfoot);
	bone(leftarm);
	bone(leftforearm);
	bone(lefthand);
	bone(rightupleg);
	bone(rightleg);
	bone(rightfoot);
	bone(rightarm);
	bone(rightforearm);
	bone(righthand);

	DrawBoneLine(r, color, head, pelvis);

	DrawBoneLine(r, color, pelvis, leftupleg);
	DrawBoneLine(r, color, leftupleg, leftleg);
	DrawBoneLine(r, color, leftleg, leftfoot);

	DrawBoneLine(r, color, pelvis, rightupleg);
	DrawBoneLine(r, color, rightupleg, rightleg);
	DrawBoneLine(r, color, rightleg, rightfoot);

	DrawBoneLine(r, color, neck, leftarm);
	DrawBoneLine(r, color, leftarm, leftforearm);
	DrawBoneLine(r, color, leftforearm, lefthand);

	DrawBoneLine(r, color, neck, rightarm);
	DrawBoneLine(r, color, rightarm, rightforearm);
	DrawBoneLine(r, color, rightforearm, righthand);
}

void Human::Test(Object* target)
{
	typedef void* (__fastcall * fnPunch)(void*, void*, Vector3* matrix);
	static const fnPunch punch = (fnPunch)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("LinkToLocalSpaceOf"));

	static auto g = CGame::Get();
	static auto cfg = Config::Get()->GetGroup(xorstr_("debug"));
	static auto velocity = cfg->GetVariable(xorstr_("velocity"));

	if (!target)
		return;

	auto crasher = ServerCrasher::Get();

	crasher->Enable();
	if (crasher->GetOriginalByte() == 0x75) {
		Vector3 derp[4];
		punch(this, target, derp);
	}
	crasher->Disable();
}

void Human::UnlinkFromLocalSpace()
{
	typedef void* (__fastcall* fnPunch)(void*, __int64&, __int64&);
	static const fnPunch punch = (fnPunch)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("UnlinkFromLocalSpace"));
	auto crasher = ServerCrasher::Get();
	__int64 as, df;
	crasher->Enable();
	if (crasher->GetOriginalByte() == 0x75) {
		punch(this, as, df);
	}
	crasher->Disable();
}

float Human::HeadDistance2D(Vector3 from)
{
	static auto head = get_bone(bone_head); 
	if (head < 0)
	{ 
		head = get_bone(bone_head);
		return 999.f;
	}

	static auto g = CGame::Get();

	auto boneWs = GetBonePositionWS(head);
	Vector3 pos = g->GetScreenPos(boneWs);

	if (pos.z < 0.1f)
		return 999.f;

	return sqrtf(powf(from.x - pos.x, 2) + powf(from.y - pos.y, 2));
}

void Human::DrawBoneLine(Renderer* r, Color color, uint32_t from, uint32_t to)
{
	static auto g = CGame::Get();

	auto v0Ws = GetBonePositionWS(from);
	auto v1ws = GetBonePositionWS(to);

	if (v0Ws.x == 0 || v0Ws.y == 0 || v0Ws.z == 0)
		return;

	if (v1ws.x == 0 || v1ws.y == 0 || v1ws.z == 0)
		return;

	Vector3 v0 = g->GetScreenPos(v0Ws);
	Vector3 v1 = g->GetScreenPos(v1ws);

	r->SetDrawColor(color);
	r->AddFilledLine({v0.x, v0.y, v1.x, v1.y});
}
