#pragma once

#include "../Singleton.h"

#define MAX_NAME_LEN 32

#define IDENTIFIER_BOOL 'b'
#define IDENTIFIER_FLOAT 'f'
#define IDENTIFIER_INT 'i'


class ConfigGroup;
class Variable;

template <typename T>
class VariableStorage;

enum VariableEvent : int
{
	GetValueString = 0,
	GetIdentifier,
	IncreaseValue,
	DecreaseValue
};

typedef const char* (*fnVariableHandler)(Variable*, int);


class Config : public Singleton<Config>
{
private:
	std::vector<ConfigGroup*> grp_list;

	wchar_t szCfg[MAX_PATH];
	void FirstTimeInitialization();
	void LoadConfig();
	bool FoundGroup(int& i, std::string buf);
	bool FoundVariable(int& i, std::string buf);
	ConfigGroup* InitGroup(int& i, std::string buf);
	Variable* InitVariable(int& i, std::string buf);
public:
	void Initialize();
	void Save();
	ConfigGroup* GetGroup(const char* groupName);
	auto GetGroups() { return grp_list; }
};

class Variable
{
private:
	template <typename T>
	VariableStorage<T>* this_storage()
	{
		return reinterpret_cast<VariableStorage<T>*>(this);
	}
	
protected:
	std::string* valueStrings;
public:
	template <typename T>
	Variable(T defaultValue, const char* variableName, fnVariableHandler value_handler, T maxValue, std::string* valueStrings)
	{
		this->value_handler = value_handler;
		SetValue<T>(defaultValue);
		SetMaxValue<T>(maxValue);
		strcpy(name, variableName);
		this->valueStrings = valueStrings;
		group = 0;
	}

	template <typename T>
	T GetValue()
	{
		return this_storage<T>()->_GetValue();
	}

	template <typename T>
	T GetMaxValue()
	{
		return this_storage<T>()->_GetMaxValue();
	}

	const char* GetValueString()
	{
		if (!value_handler)
			return "ERROR";

		return value_handler(this, ::GetValueString);
	}

	const char* GetIdentifier()
	{
		if (!value_handler)
			return "ERROR";

		return value_handler(this, ::GetIdentifier);
	}


	template <typename T>
	void SetValue(T val)
	{
		return this_storage<T>()->_SetValue(val);
	}


	template <typename T>
	void SetMaxValue(T maxVal)
	{
		return this_storage<T>()->_SetMaxValue(maxVal);
	}


	void UpdateValue(bool increase)
	{
		if (!value_handler)
			return;

		value_handler(this, increase ? IncreaseValue : DecreaseValue);
		Config::Get()->Save();
	}


	static const char* FloatHandler(Variable* var, int event)
	{
		if (event == ::GetValueString)
		{
			static char val_str[32];
			sprintf(val_str, "%.1f", var->GetValue<float>());
			return val_str;
		}

		if (event == ::GetIdentifier)
		{
			return "f";
		}

		auto cur = var->GetValue<float>();
		auto inc = 0.1f;
		auto valnew = cur;

		if (event == ::IncreaseValue)
			valnew += inc;
		else if (event == ::DecreaseValue)
			valnew -= inc;

		auto maxVal = var->GetMaxValue<float>();
		if (valnew > maxVal)
			valnew = maxVal;


		if (valnew < 0)
			valnew = 0;

		var->SetValue<float>(valnew);
		return 0;
	}

	static const char* BoolHandler(Variable* var, int event)
	{
		if (event == ::GetValueString)
		{
			if (var->valueStrings)
			{
				return var->valueStrings[(int)var->GetValue<bool>()].c_str();
			}

			static char val_str[32];
			sprintf(val_str, "%d", var->GetValue<bool>());
			return val_str;
		}

		if (event == ::GetIdentifier)
		{
			return "b";
		}

		auto cur = var->GetValue<bool>();
		var->SetValue<bool>(!cur);
		return 0;
	}

	static const char* IntHandler(Variable* var, int event)
	{
		if (event == ::GetValueString)
		{
			if (var->valueStrings)
			{
				return var->valueStrings[var->GetValue<int>()].c_str();
			}

			static char val_str[32];
			sprintf(val_str, "%i", var->GetValue<int>());
			return val_str;
		}

		if (event == ::GetIdentifier)
		{
			return "i";
		}

		auto cur = var->GetValue<int>();
		auto inc = 1;
		auto valnew = cur;

		if (event == ::IncreaseValue)
			valnew += inc;
		else if (event == ::DecreaseValue)
			valnew -= inc;

		auto maxVal = var->GetMaxValue<int>();
		if (valnew > maxVal)
			valnew = maxVal;

		if (valnew < 0)
			valnew = 0;

		var->SetValue<int>(valnew);
		return 0;
	}

	bool IsNameEqualTo(const char* s) { return strcmp(s, name) == 0; }
	char* GetName() { return name; }
	auto GetGroup() { return group; }
	void SetGroup(ConfigGroup* grp) { group = grp; }

private:
	fnVariableHandler value_handler;
	char name[MAX_NAME_LEN];
	ConfigGroup* group;
};

template <typename T>
class VariableStorage : public Variable
{
public:
	VariableStorage(T defaultValue, const char* variableName, fnVariableHandler value_handler, T maxValue, std::string* valueStrings) : Variable::Variable(defaultValue, variableName, value_handler, maxValue, valueStrings)
	{

	}
private:
	T value;
	T maxValue;
	T _GetValue() { return value; }
	void _SetValue(T val) { value = val; }
	T _GetMaxValue() { return maxValue; };
	void _SetMaxValue(T maxVal) { maxValue = maxVal; };
	friend Variable;
};

// !!!maxValue must be set for Int and Float variables!!!!
template <typename T>
static Variable* CreateVariable(T defaultValue, const char* variableName, fnVariableHandler value_handler, std::string* valueStrings = nullptr,T maxValue = (T)0)
{
	auto vs = new VariableStorage<T>(defaultValue, variableName, value_handler, maxValue, valueStrings);

	auto v = reinterpret_cast<Variable*>(vs);

	return v;
}

class ConfigGroup
{
private:
	char name[MAX_NAME_LEN];
	std::vector<Variable*> var_list;
public:
	auto GetVariables()
	{
		return var_list;
	}
	void AddVariable(Variable* var)
	{
		var->SetGroup(this);
		var_list.push_back(var);
	}

	Variable* GetVariable(const char* name)
	{
		for (auto v : var_list)
		{
			if (v->IsNameEqualTo(name))
				return v;
		}

		return 0;
	}

	ConfigGroup(const char* groupName)
	{
		strcpy(name, groupName);
	}

	bool IsNameEqualTo(const char* s) { return strcmp(s, name) == 0; }
	char* GetName() { return name; }
};