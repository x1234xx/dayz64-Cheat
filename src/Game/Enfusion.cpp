#include "../Includes.h"
#include "Enfusion.h"
#include "World.h"

EnfClassHolder::EnfClassHolder(const char* className)
{
	classPtr = FindClass(className);
}

#define SIGNATURE_ENFCLASS_GETFUNCTIONINDEXBYNAME xorstr_("E8 ? ? ? ? 89 43 7C")

Offset64 EnfClassHolder::GetFunctionByName(const char* name)
{
	if (!classPtr)
		return NULL;

	static Offset64 funcAddr = 0;
	if (!funcAddr)
	{
		auto ret = OffsetManager::Get()->PatternScan(SIGNATURE_ENFCLASS_GETFUNCTIONINDEXBYNAME);
		Offset64 func_ptr = *(Offset64*)(ret + 1);
		Offset64 func = (Offset64)ret + (Offset32)func_ptr + 5;

		funcAddr = func - 0x100000000;

		//printf("funcAddr = %llx\n", funcAddr);

	}

	typedef int(__fastcall* fnGetFuncIndexByName)(Offset64 classPtr, const char* funcName);
	static const fnGetFuncIndexByName GetFuncIndexByName = (fnGetFuncIndexByName)(funcAddr);

	int index = GetFuncIndexByName(classPtr, name);

	if (index == -1)
		return NULL;

	Offset64* v4 = (Offset64*)classPtr;

	Offset64* v9 = *(Offset64**)(v4[9] + 8 * index);

#ifdef __DEBUG_MODE
	printf("Found %s at address (%llx) and index (%i)\n", name, v9[1], index);
#endif

	return v9[1];
}

Offset64 EnfClassHolder::GetClassPtr()
{
	return classPtr;
}

inline bool& EnfClass::IsInitialized()
{
	return initialized;
}

bool EnfClass::Initialize(const char* className)
{
	if (IsInitialized())
		return true;


	enfClass = new EnfClassHolder(className);

#ifdef __DEBUG_MODE
	printf("%s initalized with class pointer: %llx\n", className, enfClass->GetClassPtr());
#endif

	initialized = true;
	return true;
}

EnfClassHolder* EnfClass::GetClass()
{
	return enfClass;
}



#define SIGNATURE_LOAD_SCRIPT xorstr_("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 48 8B F9 4C 8B C1")
#define SIGNATURE_EXEC_SCRIPT xorstr_("E8 ? ? ? ? 41 8B DF 39 5F 6C")
#define SIGNATURE_CREATE_SCRIPT_THREAD xorstr_("E8 ? ? ? ? 33 C9 44 8D 47 01")
#define SIGNATURE_GET_GLOBAL_FUNCTION xorstr_("E8 ? ? ? ? 66 FF C0")

void EnScript::Initialize()
{
	auto om =	OffsetManager::Get();

	ExecScript = (fnExecScript)(om->GetCallDestination(SIGNATURE_EXEC_SCRIPT) - 0x100000000	);
	CreateScriptThread = (fnCreateScriptThread)(om->GetCallDestination(SIGNATURE_CREATE_SCRIPT_THREAD));
	GetGlobalFunctionIndex = (fnGetGlobalFunctionIndex)(om->GetCallDestination(SIGNATURE_GET_GLOBAL_FUNCTION));
	::LoadScript = (fnLoadScript)(om->PatternScan(SIGNATURE_LOAD_SCRIPT));
/*	char t[512];
	sprintf(t, "Base: %llx\n, ExecScript: %llx\nFN_CreateScriptThread: %llx\nGetGlobalFunctionIndex: %llx\nFN_LoadScript: %llx\n", (Offset64)0, ExecScript, CreateScriptThread, GetGlobalFunctionIndex, LoadScript);

	MessageBoxA(0, t, 0, 0);*/
}

EnScript* EnScript::LoadScript(std::string path)
{
	static const auto world = World::Get();
	EnScript s;
	s.ScriptModule = ::LoadScript(world->GetScriptModule(), path.c_str());

	//previousModule = ScriptModule;
	s.ScriptContext = *(Offset64**)(s.ScriptModule + 0x80);

	s.OnStartup = GetGlobalFunctionIndex(s.ScriptModule, xorstr_("OnStartup"));
	s.OnExit = GetGlobalFunctionIndex(s.ScriptModule, xorstr_("OnExit"));

	s.ScriptThread = CreateScriptThread(s.ScriptContext, 1, s.ScriptModule);

	ExecScript(s.ScriptModule, s.ScriptThread, s.OnStartup + 1, 0, 0, 0);

	return &s;
}

void EnScript::Exit()
{
	auto ScriptThread = CreateScriptThread(ScriptContext, 1, ScriptModule);
	ExecScript(ScriptModule, ScriptThread, OnExit + 1, 0, 0, 0);
}
