#pragma once

#include "../Utils/OffsetManager.h"
#include <ctype.h>

typedef void* EnfPtr;
#define EnfCast(var, _class) reinterpret_cast<_class*>(var)

/*
Enfusion Object
*/

class EnfClassHolder
{
private:
	Offset64 classPtr;
public:
	EnfClassHolder(const char* className);
	Offset64 GetFunctionByName(const char* name);
	Offset64 GetClassPtr();
private:
	static Offset64 FindClass(const char* className);
};

/*
Enfusion Classes
*/

class EnfClass
{
protected:
	EnfClassHolder* enfClass;
	bool initialized;
public:
	bool& IsInitialized();
	virtual bool Initialize(const char* className);
	EnfClassHolder* GetClass();
};

template<typename T>
class EnfClassSingleton : public EnfClass
{
protected:
	EnfClassSingleton() {}
	~EnfClassSingleton() {}

	EnfClassSingleton(const EnfClassSingleton&) = delete;
	EnfClassSingleton& operator=(const EnfClassSingleton&) = delete;

	EnfClassSingleton(EnfClassSingleton&&) = delete;
	EnfClassSingleton& operator=(EnfClassSingleton&&) = delete;

public:
	static T* Get()
	{
		static T inst{};
		return &inst;
	}
};

class BuildingTableEntry
{
public:
	char ___pad[0x10];
	class Object* building;
};

template <typename T>
class AutoArray
{
private:
	char unknown[0x28]; // 0x00

	Offset64* m_pBuffer; // 0x28
	uint32_t m_capacity; // 0x30
	uint32_t m_size; // 0x34

public:
	uint32_t GetSize()
	{
		return m_size;
	}

	uint32_t GetCapacity()
	{
		return m_capacity;
	}

	T Get(int i)
	{
		if (!m_pBuffer)// || (i + 1) > m_size)
			return NULL;

		if (!m_pBuffer[i])
			return NULL;

		T ret = *(T*)(m_pBuffer[i] + 0x10);

		return ret;
	}

/*	template <typename _T>
	void MergeWithVector(std::vector<_T>& vec)
	{
		for (uint32_t i = 0; i < m_size; ++i)
		{
			auto a = Get(i);
			if(a)
				vec.push_back((_T)a);
		}
	}*/
};

template <typename T>
static AutoArray<T>* AllocateArray()
{
	return (AutoArray<T>*)VirtualAlloc(NULL, sizeof(AutoArray<T>), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

static BOOL FreeArray	(void* pArray)
{
	if (!pArray)
		return FALSE;

	return VirtualFree(pArray, 0, MEM_RELEASE);
}

class EString
{
public:
	uint64_t references;
	uint64_t length;
	char string[256];
	bool isValid() {

		/*
		int len = strlen(string);

		if (len > 255)
			return false;

		for (int i = 0; i < len; i++) {
			if (!isprint(string[i]))
				return false;
		}*/

		
		//if (length > 128 || !(void*)string || !length)
			//return false;

		return true;
	}

	/*std::wstring& convertToWC()
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wsPath = converter.from_bytes(string);
		return wsPath;
	}*/

	bool contains(std::string str2)
	{
		std::string str(string);

		return (str.find(str2) != std::string::npos);
	}

	bool contains(std::wstring str2)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring str = converter.from_bytes(string);

		return (str.find(str2) != std::wstring::npos);
	}
};
#define SIGNATURE_ENFCLASS_INITIALIZER xorstr_("4C 8B 05 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8B CF 48 8B 5C 24 ?? 48 83 C4 30 5F E9 ?? ?? ?? ??")

inline Offset64 EnfClassHolder::FindClass(const char* className)
{
	static auto om = OffsetManager::Get();

	Offset8* ret = 0;
	Offset64 start = 0;
	while ((ret = om->PatternScan(SIGNATURE_ENFCLASS_INITIALIZER, start)) != nullptr)
	{
		start = (Offset64)ret + 1;

		Offset64 enfclass_ptr = *(Offset64*)(ret + 3);
		Offset64 enfclass = (Offset64)ret + (Offset32)enfclass_ptr + 7;


		Offset64 string_ptr = *(Offset64*)(ret + 10);
		Offset64 string = (Offset64)ret + (Offset32)string_ptr + 14;

		char* name = (char*)string;

		if (strcmp(className, name) == 0)
			return *(Offset64*)enfclass;
	}

	//printf("ERROR CLASS NOT FOUND!\n");
	return NULL;
}

typedef Offset64(__fastcall* fnLoadScript)(Offset64 ParentModule, const char* path);
typedef int(__fastcall* fnGetGlobalFunctionIndex)(Offset64 ScriptModule, const char* funcName);
typedef Offset64* (__fastcall* fnCreateScriptThread)(Offset64* ScriptContext, char one, Offset64 ScriptModule);
typedef Offset64(__fastcall* fnExecScript)(Offset64 ScriptModule, Offset64* ScriptThread, int FunctionIndex, Offset64, Offset64, Offset64);
static fnLoadScript LoadScript;
static fnGetGlobalFunctionIndex GetGlobalFunctionIndex;
static fnCreateScriptThread CreateScriptThread;
static fnExecScript ExecScript;

class EnScript
{
protected:
	Offset64
		ScriptModule,
		* ScriptContext,
		* ScriptThread;
	int OnExit;
	int OnStartup;
	
public:
	static void Initialize();
	static EnScript* LoadScript(std::string path);
	void Exit();
};