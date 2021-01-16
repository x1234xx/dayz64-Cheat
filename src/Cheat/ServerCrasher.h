
#include "../Singleton.h"

class ServerCrasher : public Singleton<ServerCrasher>
{
private:
	char originalByte = 0;
	void GetOriginal();
	const Offset64 GetAddress();
public:
	void Enable();
	void Disable();
	void Punch();
	char GetOriginalByte();
};


