#pragma once

#include "../Includes.h"
#include "../Utils/Utils.h"

namespace Hooks
{
	inline fnPresent oPresent;
	long __stdcall hkPresent(IDXGISwapChain* p_swapchain, unsigned int syncintreval, unsigned int flags);
}