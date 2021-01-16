#pragma once

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <windows.h>
#include <string>
#include <cstdint>
#include <vector>
#include <D3D11.h>
#include <DXErr.h>
#include <D3DX11async.h>
#include <D3Dcompiler.h>
#include <D3dx11effect.h>
#include <D3D11Shader.h>
#include <DirectXMath.h>
#include <codecvt>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <fstream>
#include <mutex>

typedef DirectX::XMFLOAT3 Vector3;
typedef DirectX::XMFLOAT4 Color;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "D3DCompiler.lib")
#pragma comment (lib, "lib\\Effects11.lib")
//#pragma comment(lib, "dxerr.lib")
//#pragma comment(lib, "legacy_stdio_definitions.lib")

#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#include "Utils/XorStr.h"


//#define __DEBUG_MODE