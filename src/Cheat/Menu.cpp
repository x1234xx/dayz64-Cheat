#include "../Includes.h"
#include "../Utils/Utils.h"

#include "../Utils/D3D11/Renderer.h"
#include "Config.h"

#include "Menu.h"

void Menu::Initialize()
{

}

void Menu::DrawGroup(ConfigGroup* grp, int y_mlptr, bool selected)
{
	auto label = Renderer::Get()->PrepareString(L"[%S]", grp->GetName());
	Renderer::Get()->Text(20, 10 + (y_mlptr * 24), label, selected ? Color(150, 200, 255, 255) : Color(255, 255, 255, 255));

}

void Menu::DrawVariable(Variable* var, int y_mlptr, bool selected)
{
	auto label = Renderer::Get()->PrepareString(L"* [%S] = %S", var->GetName(), var->GetValueString());
	Renderer::Get()->Text(20, 10 + (y_mlptr * 24), label, selected ? Color(150, 200, 255, 255) : Color(255, 255, 255, 255));
}


void Menu::Draw()
{
	static ConfigGroup* grp_sel = 0;

	int count_sel = 0;

	grp_sel == 0 ? (count_sel = Config::Get()->GetGroups().size()) : (count_sel = grp_sel->GetVariables().size() + 1);

	count_sel -= 1;

	static int index = 0;
	int draw_count = 0;

	if (Utils::Get()->IsKeyDown(VK_UP))
	{
		--index;
	}

	if (Utils::Get()->IsKeyDown(VK_DOWN))
	{
		++index;
	}

	if (index < 0)
		index = count_sel;

	if (index > count_sel)
		index = 0;


	if (Utils::Get()->IsKeyDown(VK_RIGHT))
	{
		if (!grp_sel)
		{
			grp_sel = Config::Get()->GetGroups().at(index);
			index = 0;
		}
		else if (grp_sel && index > 0)
		{
			auto var = grp_sel->GetVariables().at(index - 1);
			if (var)
				var->UpdateValue(true);
		}
	}

	if (Utils::Get()->IsKeyDown(VK_LEFT))
	{
		if (index == 0 && grp_sel != 0)
			grp_sel = 0;
		else if (grp_sel && index > 0)
		{
			auto var = grp_sel->GetVariables().at(index - 1);
			if (var)
				var->UpdateValue(false);
		}
	}

	if (!grp_sel)
	{
		for (auto grp : Config::Get()->GetGroups())
		{
			bool selected = draw_count == index;

			DrawGroup(grp, draw_count, selected);

			++draw_count;
		}
	}
	else
	{
		DrawGroup(grp_sel, 0, index == 0);

		++draw_count;

		for (auto var : grp_sel->GetVariables())
		{
			bool selected = (draw_count == index);

			DrawVariable(var, draw_count, selected);

			++draw_count;
		}
	}
}