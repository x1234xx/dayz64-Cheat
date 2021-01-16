#include "../Includes.h"
#include "Object.h"
#include "CGame.h"
#include "Human.h"	
#include "HumanInventory.h"
#include "EntityAI.h"
#include "../Utils/D3D11/Renderer.h"
#include "../Cheat/Config.h"
#include "../Utils/Utils.h"
#include "../Cheat/ServerCrasher.h"
#include <algorithm>
#include "../Cheat/MurderMode.h"
#include "../Cheat/ItemHighlighter.h"


#define TYPE_PLAYER  ("dayzplayer")
#define TYPE_ZOMBIE  ("dayzinfected")
#define TYPE_ANIMAL  ("dayzanimal")
#define TYPE_INVITEM ("inventoryItem")
#define TYPE_WHEEL  ("carwheel")
#define TYPE_CLOTHING  ("clothing")
#define TYPE_WATCH  ("itemwatch")
#define TYPE_RADIO  ("itemradio")
#define TYPE_OPTICS  ("itemoptics")
#define TYPE_BARREL  ("itembarrel")
#define TYPE_SUPRESSOR  ("ItemSuppressor")
#define TYPE_GRENADE  ("handgrenade")
#define TYPE_WEAPON ("Weapon")
#define TYPE_PROXYMAGAZINES ("ProxyMagazines")
#define TYPE_CAR ("car")

#define ISTYPEOF(a, b) !strcmp(a, b)

Vector3& Object::GetPosition()
{
	Vector3 vec(0, 0, 0);

	typedef Vector3&(__fastcall* fnGetPosition)(Object* ent, Vector3* out);
	static const fnGetPosition getPosition = (fnGetPosition)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("GetPosition"));

	if (!getPosition) {
		return vec;
	}



#ifdef __DEBUG_MODE
	//printf("getPosition: %llx\n return = { %.3f | %.3f | %.3f }\n", (Offset64)getPosition, vec.x, vec.y, vec.z);
#endif
	getPosition(this, &vec);

	return vec;
}

void Object::SetPosition(Vector3 pos)
{
	typedef void*(__fastcall* fnSetPosition)(Offset64 ent, Vector3 pos);
	static const fnSetPosition setPosition = (fnSetPosition)enfClass->Get()->GetClass()->GetFunctionByName(xorstr_("SetPosition"));

	if (!setPosition)
		return;

	setPosition((Offset64)this, pos);

#ifdef __DEBUG_MODE
	//printf("setPosition: %llx\n return = { %.3f | %.3f | %.3f }\n", (Offset64)setPosition, pos.x, pos.y, pos.z);
#endif

	return;
}

EString* Object::GetType()
{
	static auto om = OffsetManager::Get();
	Offset64 typeClass = GetTypeClass();

	if (!typeClass)
		return NULL;

	void* typeStr = *(void**)(typeClass + 0xA0);

	if (!typeStr)//om->IsBadReadPtr(typeStr))
		return NULL;

	return reinterpret_cast<EString*>(typeStr);
}

EString* Object::GetCleanName()
{
	/*auto var = this->GetConfigVariable("displayName");

	//char t[256];
	auto ptr = *(Offset64*)((Offset64)var + 0x8);

	Offset64* _ptr = NULL;
	if (ptr) 
		_ptr = (Offset64*)(ptr + 0x10);

	static auto om = OffsetManager::Get();

	EString* str = NULL;
	if (_ptr && !om->IsBadReadPtr((void*)*_ptr)) 
	{
		str = (EString*)*_ptr;
	}*/

	//if (!str || !strncmp(str->string, "$STR", 4))
	//{
		Offset64 typeClass = GetTypeClass();

		if (typeClass == NULL)
			return NULL;

		void* nameStr = *(void**)(typeClass + 0x4D0);

		if (!nameStr)//om->IsBadReadPtr(nameStr))
			return NULL;

		return reinterpret_cast<EString*>(nameStr);
	//}

	//return str;
}

EString* Object::GetModelPath()
{
	static auto om = OffsetManager::Get();
	Offset64 typeClass = GetTypeClass();

	if (!typeClass)
		return NULL;

	void* typeStr = *(void**)(typeClass + 0x80);

	if (!typeStr)//om->IsBadReadPtr(typeStr))
		return NULL;

	return reinterpret_cast<EString*>(typeStr);
}

EString* Object::GetConfigName()
{
	static auto om = OffsetManager::Get();
	Offset64 typeClass = GetTypeClass();

	if (typeClass == NULL)
		return NULL;

	void* nameStr = *(void**)(typeClass + 0x68);

	if (!nameStr)//om->IsBadReadPtr(nameStr))
		return NULL;

	return reinterpret_cast<EString*>(nameStr);
}

void Object::Draw(Object* localPlayer, Vector3& localPosition)
{
	static auto g = CGame::Get();
	static auto mm = MurderMode::Get();
	//static auto ihl = ItemHighlighter::Get();
	static auto cfg = Config::Get()->GetGroup(xorstr_("visuals"));
	static auto player = cfg->GetVariable(xorstr_("player_esp"));
	static auto corpse = cfg->GetVariable(xorstr_("corpse_esp"));
	static auto zombie = cfg->GetVariable(xorstr_("zombie_esp"));
	static auto animal = cfg->GetVariable(xorstr_("animal_esp"));
	static auto item_supplies = cfg->GetVariable(xorstr_("item_supplies_esp"));
	static auto item_clothing = cfg->GetVariable(xorstr_("item_clothing_esp"));
	static auto item_weapon = cfg->GetVariable(xorstr_("item_weapon_esp"));
	static auto item_other = cfg->GetVariable(xorstr_("item_other_esp"));
	static auto car = cfg->GetVariable(xorstr_("car_esp"));
	static auto base_esp = cfg->GetVariable(xorstr_("base_esp"));
	static auto missing = cfg->GetVariable(xorstr_("missing_esp"));
	static auto helicrash = cfg->GetVariable(xorstr_("helicrash_esp"));
	static auto stash = cfg->GetVariable(xorstr_("stash_esp"));

	if (localPlayer == this)
		return;
	
	static Renderer* r = Renderer::Get();

	auto type = GetType();

	if (type == NULL)
		return;

	if (!type->isValid())
		return;


	if (missing->GetValue<bool>()) {

		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		auto model = GetModelPath();

		if (!model)
			return;

		if (v.z < 0.1f)
			return;

		return r->Text(v.x, v.y, model->string, Color(0, 255, 0, 255), true);
	}

	auto name = GetCleanName();
	if (name == NULL) { // Is Building (most likely)
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;
		auto model = GetModelPath();

		if (!model)
			return;

		if (helicrash->GetValue<bool>() && (model->contains("wreck_mi8") || model->contains("wreck_uh1y")))
		{
			auto printStr = r->PrepareString(L"Helicrash Site [%.1fm]", this->DistanceFrom(localPosition));

			return r->Text(v.x, v.y, printStr, Color(209, 105, 224, 255), true);
		}
		return;
	}

	if (!name->isValid())
		return;

	static const Color medical(255, 200, 120, 255);
	static const Color food(255, 200, 255, 255);
	static const Color backpack(255, 200, 0, 255);
	static const Color base(200, 25, 0, 255);
	static const Color highlighted(0, 255, 255, 255);
	static const Color weapon(170, 0, 255, 255);
	static const Color other(0, 114, 128, 255);

	if ((player->GetValue<bool>() || corpse->GetValue<bool>()) && ISTYPEOF(type->string, TYPE_PLAYER))
	{
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;


		bool isDead = this->IsDead();
		bool isTarget = this == mm->GetTarget();

		Color bone_color = Color(255, 255, 255, 255);
		Color text_color = Color(0, 100, 255, 255);

		auto id = this->GetNetworkId();
		auto playername = g->GetPlayerName(id);
		if (isDead)
		{
			if (!corpse->GetValue<bool>())
				return;

			bone_color = text_color = Color(100, 100, 100, 255);
			playername = "Corpse";
		}

		auto human = (Human*)this;
		human->Draw(r, bone_color);


		Color textColor = isTarget ? Color(0, 225, 255, 255) : text_color;

		auto objInHands = this->GetEntityInHands();
		if (objInHands) {
			auto objName = objInHands->GetCleanName();
			if (objName && objName->isValid()) {
				r->Text(v.x, v.y - 20, objName->string, textColor, true);
			}
		}
		else r->Text(v.x, v.y - 20, "none", textColor, true);

		auto dist = this->DistanceFrom(localPosition);
		auto printStr = r->PrepareString("%s [%.1fm]", playername, dist);

		return r->Text(v.x, v.y, printStr, textColor, true);
	}
	if (zombie->GetValue<bool>() && ISTYPEOF(type->string, TYPE_ZOMBIE))
	{
		if (this->IsDead())
			return;

		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;

		float distance = this->DistanceFrom(localPosition);
	
		if (distance > 100)
			return;

		auto printStr = r->PrepareString(L"%S [%.1fm]", name->string, distance);

		return r->Text(v.x, v.y, printStr, Color(255, 0, 0, 255), true);
	}

	if (animal->GetValue<bool>() && ISTYPEOF(type->string, TYPE_ANIMAL))
	{

		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;
		auto printStr = r->PrepareString("%s [%.1fm]", name->string, this->DistanceFrom(localPosition));

		return r->Text(v.x, v.y, printStr, Color(35, 155, 155, 255), true);
	}

	if (car->GetValue<bool>() && (ISTYPEOF(type->string, TYPE_CAR)
		|| ISTYPEOF(type->string, TYPE_WATCH) || ISTYPEOF(type->string, TYPE_RADIO))) {
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;
		auto printStr = r->PrepareString("%s [%.1fm]", name->string, this->DistanceFrom(localPosition));

		return r->Text(v.x, v.y, printStr, Color(209, 105, 224, 255), true);
	}

	bool supplies_enabled = item_supplies->GetValue<bool>();
	bool base_enabled = base_esp->GetValue<bool>();
	bool stash_enabled = stash->GetValue<bool>();
	bool isHighlighted = false;// ihl->ShouldHighlight(name);

	if ((base_enabled || supplies_enabled || stash_enabled) && (ISTYPEOF(type->string, TYPE_INVITEM)))
	{
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;

		auto printStr = r->PrepareString("%s [%.1fm]", name->string, this->DistanceFrom(localPosition));

		Color color = Color(0, 114, 128, 255);
		auto model = GetModelPath();

		if (!model)
			return;

		if (stash_enabled && model->contains("underground_stash")) {
			return r->Text(v.x, v.y, printStr, Color(200, 100, 0, 255), true);
		}

		if (model->contains("medical")) {
			color = medical;
		}

		if (model->contains("containers")) {
			color = backpack;
		}

		if (model->contains("food") || 
			model->contains("drinks")) {
			color = food;
		}
		
		bool isBaseItem = false;
		if ((model->contains("furniture") || model->contains("camping") || !strcmp(name->string, "Barrel")) 
			&& !model->contains("kit")) {
			isBaseItem = true;
			color = base;
		}

		if (isHighlighted)
			color = highlighted;

		if ((isBaseItem && base_enabled) || (!isBaseItem && supplies_enabled))
			return r->Text(v.x, v.y, printStr, color, true);
		else 
			return;
	}
	if (item_clothing->GetValue<bool>() && (ISTYPEOF(type->string, TYPE_CLOTHING))) 
	{
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		if (v.z < 0.1f)
			return;
		auto printStr = r->PrepareString("%s [%.1fm]", name->string, this->DistanceFrom(localPosition));

		Color color = Color(0, 114, 128, 255);

		auto model = GetModelPath();

		if (model->contains("backpacks")) {
			color = backpack;
		}
		if (isHighlighted)
			color = highlighted;
		return r->Text(v.x, v.y, printStr, color, true);
	}

	if (item_weapon->GetValue<bool>() && (ISTYPEOF(type->string, TYPE_OPTICS) || ISTYPEOF(type->string, TYPE_SUPRESSOR) 
		|| ISTYPEOF(type->string, TYPE_GRENADE) || ISTYPEOF(type->string, TYPE_WEAPON) || ISTYPEOF(type->string, TYPE_PROXYMAGAZINES)))
	{
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		Color color = weapon;
		
		if (v.z < 0.1f)
			return;
		auto printStr =  r->PrepareString("%s [%.1fm]", name->string, this->DistanceFrom(localPosition));
		if (isHighlighted)
			color = highlighted;

		return r->Text(v.x, v.y, printStr, color, true);
	}

	if (item_other->GetValue<bool>() && (ISTYPEOF(type->string, TYPE_BARREL) || ISTYPEOF(type->string, TYPE_WHEEL)))
	{
		auto pos = this->GetPosition();
		auto v = g->GetScreenPos(pos);

		Color color = other;

		if (v.z < 0.1f)
			return;
		auto printStr = r->PrepareString("%s [%.1fm]", name->string, this->DistanceFrom(localPosition));

		if (isHighlighted)
			color = highlighted;

		return r->Text(v.x, v.y, printStr, color, true);
	}

}

float Object::DistanceFrom(Vector3& from)
{
	Vector3 pos = this->GetPosition();
	return sqrtf(powf(from.x - pos.x, 2) + powf(from.y - pos.y, 2) + powf(from.z - pos.z, 2));
}

float Object::DistanceFrom2D(Vector3& from2d)
{
	static auto g = CGame::Get();
	Vector3 pos = g->GetScreenPos(this->GetPosition());
	return sqrtf(powf(from2d.x - pos.x, 2) + powf(from2d.y - pos.y, 2));
}

void Object::TestDamage(Object* target)
{
	typedef void(__fastcall * tCloseCombatDamage)(void*, void*, int, void*, Vector3);
	static const auto dmg = (tCloseCombatDamage)((Offset64)GetModuleHandle(NULL) + 0x3AA260);
	static const auto testSlash = *(EString**)((Offset64)GetModuleHandle(NULL) + 0xDD0210);
	static const auto adr = (Offset64)GetModuleHandle(NULL) + 0x5E8D67;

	Human* h = (Human*)target;
	int testidx = h->GetBoneIndexByName(bone_pelvis);
	Vector3 test = h->GetBonePositionWS(testidx);

	dmg(this, target, 1, testSlash, test);
}

HumanInventory* Object::GetInventory()
{
	return EnfCast(this, EntityAI)->GetInventory();
}

Object* Object::GetEntityInHands()
{
	auto inv = this->GetInventory();
	if (inv)
		return inv->GetEntityInHands();
	
	return NULL;
}

Offset64* Object::GetConfigVariable(const char* entryName)
{
	typedef Offset64* (__fastcall* fnGetVar)(Offset64* out, Object* obj, const char* name);
	static fnGetVar getVar = (fnGetVar)((Offset64)GetModuleHandle(NULL) + 0x4C9430);

	Offset64 var[2];
	getVar(var, this, entryName);

	return var;
}

bool Object::IsKindaBarrel()
{
	auto type = GetType();

	if (type == NULL)
		return false;

	if (!type->isValid())
		return false;

	auto model = GetModelPath();

	if (ISTYPEOF(type->string, TYPE_INVITEM))
	{

		if (model->contains("containers")) {
			return true;
		}

	}

	return false;
}

int Object::GetNetworkId()
{
	return *reinterpret_cast<int*>((Offset64)this + 0x59C);
}

bool Object::IsDead()
{
	return *reinterpret_cast<bool*>((Offset64)this + 0xF5);
}

Offset64 Object::GetTypeClass()
{
	static auto om = OffsetManager::Get();
	void* typeClass =  *(void**)((Offset64)this + 0xE0);

	if (!typeClass)//om->IsBadReadPtr(typeClass))
		return NULL;

	return reinterpret_cast<Offset64>(typeClass);
}
