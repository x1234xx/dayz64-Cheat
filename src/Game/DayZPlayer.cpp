#include "../Includes.h"
#include "DayZPlayer.h"
#include "Object.h"
#include "Human.h"
#include "../Utils/Utils.h"

void DayZPlayer::PunchTarget(Object* target)
{
	if (!target)
		return;

	//void __fastcall sub_14043C1E0(_QWORD *a1, __int64 a2, unsigned int a3, __int64 a4, unsigned int a5, int *a6)
	typedef void(__fastcall * fnProcessMeleeHit)(DayZPlayer*, void*, int, Object*, int, Vector3);
	static const auto processMelee = reinterpret_cast<fnProcessMeleeHit>(enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("ProcessMeleeHit")));
	Human* h = (Human*)target;
	int testidx = h->GetBoneIndexByName(bone_pelvis);
	Vector3 test = h->GetBonePositionWS(testidx);

	processMelee(this, 0, 1, target, testidx, test);

}

