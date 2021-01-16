#pragma once

#include "../Singleton.h"
#include "../Includes.h"


typedef std::uint8_t	Offset8;
typedef std::uint16_t	Offset16;
typedef std::uint32_t	Offset32;
typedef std::uint64_t	Offset64;

class OffsetManager : public Singleton<OffsetManager>
{
public:
	std::uint8_t* PatternScan(const char* signature, Offset64 start = 0, void* module = GetModuleHandle(NULL));
	bool IsBadReadPtr(void* p);
	Offset64 GetCallDestination(const char* signature, Offset64 start = 0, void* module = GetModuleHandle(NULL));
};