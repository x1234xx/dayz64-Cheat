
#include "../Singleton.h"
#include "../Game/Enfusion.h"
#include "../Game/Object.h"

class Speedhack : public Singleton<Speedhack>
{
private:
	__int64 OriginalFreq = 0;
	Offset64 Freq;
	void GetOriginal();
public:
	void Enable();
	void Disable();
};


