#pragma once

#include "../Singleton.h"

class ConfigGroup;
class Variable;

class Menu : public Singleton<Menu>
{
public:
	void Initialize();
	void DrawGroup(ConfigGroup* grp, int y_mlptr, bool selected);
	void DrawVariable(Variable* grp, int y_mlptr, bool selected);
	void Draw();
};