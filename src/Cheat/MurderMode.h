#pragma once

#include "../Singleton.h"
#include "../Game/Enfusion.h"
#include "../Game/Object.h"

class MurderMode : public Singleton<MurderMode>
{
private:
	std::vector<Object*> players;
	Object* target;
	HWND gameWindow;
public:
	void Initialize();
	Object* GetTarget();
	void KillTarget();
	void Update();
	void Add(Object* player);
};