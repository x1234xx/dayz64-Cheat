#include "HumanInputController.h"

void HumanInputController::SetDisabled(bool state)
{
	typedef void(__fastcall * fnSetDisabled)(void*, bool);
	static const fnSetDisabled setDisabled = (fnSetDisabled)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("SetDisabled"));

	if (!setDisabled)
		return;

	setDisabled(this, state);
}
