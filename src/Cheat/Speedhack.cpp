#include "Speedhack.h"
#include "Config.h"

#define SIGNATURE_FREQ xorstr_("48 83 EC 28 48 83 3D ?? ?? ?? ?? ?? 75 3B")

void Speedhack::GetOriginal()
{
	if (!Freq)
	{
		auto ret = OffsetManager::Get()->PatternScan(SIGNATURE_FREQ);
		Offset64 freq_ptr = *(Offset64*)(ret + 28);
		Freq = (Offset64)ret + (Offset32)freq_ptr + 0x20;
	}
	OriginalFreq = *(__int64*)Freq;
}

void Speedhack::Enable()
{
	if (!OriginalFreq)
		GetOriginal();
	
	static auto cfg = Config::Get()->GetGroup(xorstr_("misc"));
	static auto speed = cfg->GetVariable(xorstr_("speedhack_speed"));
		
	*(__int64*)(Freq) = OriginalFreq / (1 + speed->GetValue<int>());
}

void Speedhack::Disable()
{
	if (!OriginalFreq)
		GetOriginal();

	*(__int64*)(Freq) = OriginalFreq;
}
