#include "../Includes.h"
#include "Config.h"
#include <Shlobj.h>

std::string on_off[] = { xorstr_("off"), xorstr_("on") };
std::string bone_list[] = { xorstr_("head"), xorstr_("chest") };

void Config::FirstTimeInitialization()
{
	

	auto visGrp = new ConfigGroup(xorstr_("visuals"));
	visGrp->AddVariable(CreateVariable(false, xorstr_("player_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("corpse_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("zombie_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("animal_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("item_supplies_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("item_weapon_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("item_clothing_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("item_other_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("car_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("base_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("helicrash_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("stash_esp"), Variable::BoolHandler, on_off));
	visGrp->AddVariable(CreateVariable(false, xorstr_("missing_esp"), Variable::BoolHandler, on_off));
	grp_list.push_back(visGrp);


	auto miscGrp = new ConfigGroup(xorstr_("misc"));
#ifdef RISKY_FEATURES
	miscGrp->AddVariable(CreateVariable(false, xorstr_("speedhack"), Variable::BoolHandler, on_off));
	miscGrp->AddVariable(CreateVariable(4, xorstr_("speedhack_speed"), Variable::IntHandler, nullptr, 8));
#endif
	miscGrp->AddVariable(CreateVariable(false, xorstr_("murdermode"), Variable::BoolHandler, on_off));
	miscGrp->AddVariable(CreateVariable(0, xorstr_("mm_bone"), Variable::IntHandler, bone_list, 1));
	miscGrp->AddVariable(CreateVariable(100, xorstr_("mm_radius"), Variable::IntHandler, nullptr, 300));
	miscGrp->AddVariable(CreateVariable(false, xorstr_("mm_draw_radius"), Variable::BoolHandler, on_off));
	miscGrp->AddVariable(CreateVariable(false, xorstr_("freecam"), Variable::BoolHandler, on_off));
	miscGrp->AddVariable(CreateVariable(false, xorstr_("print_position"), Variable::BoolHandler, on_off));
//	miscGrp->AddVariable(CreateVariable(false, xorstr_("servercrasher"), Variable::BoolHandler));
	grp_list.push_back(miscGrp);


	//auto testGrp = new ConfigGroup(xorstr_("debug"));
//	testGrp->AddVariable(CreateVariable(0, xorstr_("velocity"), Variable::IntHandler, 100));
	//grp_list.push_back(testGrp);

	Save();
}

void Config::Initialize()
{
	return FirstTimeInitialization();
	PWSTR szAppData;
	wchar_t szFolder[MAX_PATH];
	if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, 0, &szAppData)))
	{
		return;
	}
	
	swprintf(szFolder, xorstr_(L"%s\\CheatsBuddy"), szAppData);
	swprintf(szCfg, xorstr_(L"%s\\dz.bcfg"), szFolder);

	std::ifstream f(szCfg);

	if (CreateDirectoryW(szFolder, 0) || !f.good())
	{
		f.close();
		std::ofstream n;
		n.open(szCfg, std::ofstream::out);
		n.close();
		return FirstTimeInitialization();
	}
	f.close();

	return LoadConfig();
}

void Config::LoadConfig()
{
	std::string buf;

	std::ifstream f(szCfg);
	f.seekg(0, std::ios::end);
	buf.resize(f.tellg());
	f.seekg(0);
	f.read(buf.data(), buf.size());
	f.close();

	int i = 0;
	ConfigGroup* grp = 0;
	while (i < buf.size())
	{
		if (FoundGroup(i, buf))
		{
			grp = InitGroup(i, buf);
			grp_list.push_back(grp);
		}

		if (FoundVariable(i, buf))
		{
			auto v = InitVariable(i, buf);
			if (v)
				grp->AddVariable(v);
		}

		++i;
	}

}

void Config::Save()
{
	std::string buf;
	std::ofstream f(szCfg);

	for (auto g : grp_list)
	{
		char grp[MAX_NAME_LEN + 1];
		sprintf(grp, "[%s]\n", g->GetName());
		buf += grp;
		for (auto v : g->GetVariables())
		{
			char var[32];
			sprintf(var, "*%s = %s%s;\n", v->GetName(), v->GetValueString(), v->GetIdentifier());
			buf += var;
		}
		buf += '\n';
	}

	f.write(buf.c_str(), buf.size());

	f.close();
}

ConfigGroup* Config::GetGroup(const char* groupName)
{
	for (auto g : grp_list)
	{
		if (g->IsNameEqualTo(groupName))
			return g;
	}

	return NULL;
}


bool Config::FoundGroup(int& i, std::string buf)
{
	return (buf[i] == '[');
}

ConfigGroup* Config::InitGroup(int& i, std::string buf)
{
	++i;

	std::string grpName;
	while (buf[i] != ']' && i < buf.size())
	{
		grpName.push_back(buf[i]);
		++i;
	}

	return new ConfigGroup((char*)grpName.c_str());
}

bool Config::FoundVariable(int& i, std::string buf)
{
	return (buf[i] == '*');
}

Variable* Config::InitVariable(int& i, std::string buf)
{
	std::string varName;
	std::string varVal;
	std::string varType;
	bool collectingName = true;

	while (buf[i] != ';' && i < buf.size())
	{
		if (collectingName)
		{
			if (buf[i] == '=')
			{
				collectingName = false;
			}
			else if ((buf[i] >= 'a' && buf[i] <= 'z') || buf[i] == '_')
			{
				varName.push_back(buf[i]);
			}
		}
		else if ((buf[i] >= '0' && buf[i] <= '9') || buf[i] == '.')
		{
			varVal.push_back(buf[i]);
		}
		else
		{
			if (buf[i] == IDENTIFIER_BOOL)
			{
				return ::CreateVariable((bool)atoi(varVal.c_str()), (char*)varName.c_str(), Variable::BoolHandler);
			}

			if (buf[i] == IDENTIFIER_FLOAT)
			{
				return ::CreateVariable((float)atof(varVal.c_str()), (char*)varName.c_str(), Variable::FloatHandler);
			}

			if (buf[i] == IDENTIFIER_INT)
			{
				return ::CreateVariable(atoi(varVal.c_str()), (char*)varName.c_str(), Variable::IntHandler);
			}
		}
		++i;
	}

	return NULL;
}