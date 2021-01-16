#include "Renderer.h"

#include <chrono>
#include <system_error>

#include <random>
using namespace DirectX;
using namespace std::chrono;

#define CALC_FONT_SIZE(a) -MulDiv( a, (INT)(GetDeviceCaps(hDC, LOGPIXELSY)), 72 )

HRESULT FontData::CreateFontObjects(const Font font, FontData* data, std::unique_ptr<DWORD[]>& lpBitmapBits)
{
	DWORD texWidth, texHeight;
	if (font.Size > 60)
		texWidth = 2048;
	else if (font.Size > 30)
		texWidth = 1024;
	else if (font.Size > 15)
		texWidth = 512;
	else
		texWidth = 256;

	DWORD* pBitmapBits;
	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = texWidth;
	bmi.bmiHeader.biHeight = -static_cast<int>(texWidth);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;

	HDC hDC = CreateCompatibleDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
	SetMapMode(hDC, MM_TEXT);

	HFONT hFont = CreateFontW(CALC_FONT_SIZE(font.Size), 0, 0, 0, font.Flags, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, font.FontName.c_str());

	if (!hFont)
		return E_FAIL;

	SelectObject(hDC, hBitmap);
	SelectObject(hDC, hFont);

	SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
	SetBkColor(hDC, 0x00000000);
	SetTextAlign(hDC, TA_TOP);

	float x = 0, y = 0;
	wchar_t str[2] = L"x";
	SIZE sz;

	GetTextExtentPoint32W(hDC, L" ", 1, &sz);

	float spacing = sz.cx;

	std::vector<XMFLOAT4> sizes;

	for (wchar_t c = 32; c < MAX_WCHAR; c++)
	{
		str[0] = c;
		GetTextExtentPoint32W(hDC, str, 1, &sz);

		if (x + sz.cx > texWidth)
		{
			x = 0;
			y += sz.cy;
		}

		sizes.push_back(XMFLOAT4(x, y, x + sz.cx, y + sz.cy));

		x += sz.cx + spacing / 2;
	}

	texHeight = y + sz.cy;

	wchar_t c = 0;
	for (auto & var : sizes)
	{
		str[0] = c + 32;

		ExtTextOutW(hDC, static_cast<int>(ceilf(var.x)), static_cast<int>(ceilf(var.y)), ETO_OPAQUE, NULL, str, 1, NULL);

		data->m_fTexCoords[c++] = { var.x / texWidth, var.y / texHeight, var.z / texWidth, var.w / texHeight };
	}

	data->m_Spacing = spacing;
	data->m_TexHeight = texHeight;
	data->m_TexWidth = texWidth;

	DWORD len = texWidth * texHeight;
	auto buffer = std::make_unique<DWORD[]>(len);

	if (!buffer)
		return E_OUTOFMEMORY;

	memcpy((void*)buffer.get(), pBitmapBits, min(len * sizeof(DWORD), texWidth * texWidth * sizeof(DWORD)));

	lpBitmapBits = std::move(buffer);

	DeleteObject(hBitmap);
	DeleteObject(hFont);
	DeleteDC(hDC);
	return S_OK;
}
#pragma region Shaders
#define shdr(str) std::string(xorstr_(str))


std::string random_string(std::size_t length)
{
	const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

	std::string random_string;

	for (std::size_t i = 0; i < length; ++i)
	{
		random_string += CHARACTERS[distribution(generator)];
	}

	return random_string;
}

#define RSTRING(name) const auto name = random_string(10) + ";\n"

RSTRING(POSITION);
RSTRING(COLOR);
RSTRING(TEXCOORD);

const std::string pixShader = 
	shdr("Texture2D tex2D;\n") +
	shdr("SamplerState linearSampler\n") +
	shdr("{\n") +
	shdr("	Filter = MIN_MAG_MIP_LINEAR;\n") +
	shdr("	AddressU = D3D11_TEXTURE_ADDRESS_BORDER;\n") +
	shdr("	AddressV = D3D11_TEXTURE_ADDRESS_BORDER;\n") +
	shdr("	BorderColor = float4(0.f, 0.f, 0.f, 1.f);\n") +
	shdr("};\n") + 
	shdr("struct PS_INPUT\n") +
	shdr("{\n") +
	shdr("	float4 pos : SV_POSITION;\n") +
	shdr("	float4 col : ") + COLOR +
	shdr("};\n") +
	shdr("float4 main( PS_INPUT input ) : SV_Target\n") +
	shdr("{\n") +
	shdr("	return input.col;\n") +
	shdr("};\n") +
	shdr("struct FPS_INPUT\n") +
	shdr("{\n") +
	shdr("	float4 pos : SV_POSITION;\n") +
	shdr("	float4 col : ") + COLOR +
	shdr("	float2 tex : ") + TEXCOORD +
	shdr("};\n") +
	shdr("float4 fontmain( FPS_INPUT input ) : SV_Target\n") +
	shdr("{\n") +
	shdr("	return tex2D.Sample( linearSampler, input.tex ) * input.col;\n") +
	shdr("};\n");

const  std::string  vertShader =
	shdr("struct VS_INPUT\n") +
	shdr("{\n") +
	shdr("	float4 pos : ") + POSITION +
	shdr("	float4 col : ") + COLOR + 
	shdr("};\n") +
	shdr("struct PS_INPUT\n") +
	shdr("{\n") +
	shdr("	float4 pos : SV_POSITION;\n")+
	shdr("	float4 col : ") + COLOR +
	shdr("};\n") +
	shdr("struct FVS_INPUT\n") +
	shdr("{\n") +
	shdr("	float4 pos : ") + POSITION + 
	shdr("	float4 col : ") + COLOR + 
	shdr("	float2 tex : ") + TEXCOORD +
	shdr("};\n") +
	shdr("struct FPS_INPUT\n") +
	shdr("{\n") +
	shdr("	float4 pos : SV_POSITION;\n") +
	shdr("	float4 col : ") + COLOR +
	shdr("	float2 tex : ") + TEXCOORD +
	shdr("};\n") +
	shdr("float4 rearrange(float4 value)\n") +
	shdr("{\n") +
	shdr("	float4 color;\n") +
	shdr("	color.a = value.a;\n") +
	shdr("	color.r = value.b;\n") +
	shdr("	color.g = value.g;\n") +
	shdr("	color.b = value.r;\n") +
	shdr("	return color;\n") +
	shdr("};\n") +
	shdr("PS_INPUT main( VS_INPUT input )\n") +
	shdr("{\n") +
	shdr("	PS_INPUT output;\n") +
	shdr("	output.pos = input.pos;\n") +
	shdr("	output.col = rearrange(input.col);\n") +
	shdr("	return output;\n") +
	shdr("};\n") +
	shdr("FPS_INPUT fontmain( FVS_INPUT input )\n") +
	shdr("{\n") +
	shdr("	FPS_INPUT output;\n") +
	shdr("	output.pos = input.pos;\n") +
	shdr("	output.col = rearrange(input.col);\n") +
	shdr("	output.tex = input.tex;\n") +
	shdr("	return output;\n") +
	shdr("};\n");

inline HRESULT CompileShaderFromMemory(const char* szdata, SIZE_T len, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromMemory(szdata, len, NULL, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);

	//MessageBoxA(0, szdata, 0, 0);

	if (FAILED(hr))
	{
	//	MessageBoxW(0, DXGetErrorDescription(hr), DXGetErrorString(hr), 0);
		
		if (pErrorBlob)
			pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}
#pragma endregion

void Renderer::DestroyObjects()
{
	alive = false;
	SafeRelease(m_InputLayout);
	SafeRelease(m_pshader);
	SafeRelease(m_vshader);
	SafeRelease(m_fontpshader);
	SafeRelease(m_fontvshader);

	SafeRelease(m_blendstate);
	SafeRelease(m_raststate);
}


void Renderer::Init(IDXGISwapChain* pSwapChain)
{
	alive = false;

	pSwapChain->GetDevice(__uuidof(m_pDevice), reinterpret_cast<void**>(&m_pDevice));
	ID3DBlob* pVSBlob = NULL;

	m_pDevice->GetImmediateContext(&m_pContext);

	if (FAILED(CompileShaderFromMemory(vertShader.c_str(), vertShader.size(), xorstr_("main"), xorstr_("vs_4_0"), &pVSBlob)))
	{
		DestroyObjects();
		return;
	}

	if (FAILED(m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_vshader)))
	{
		DestroyObjects();
		pVSBlob->Release();
		return;
	}

	char* pos = (char*)POSITION.c_str();
	pos[strlen(pos) - 2] = '\0';

	char* col = (char*)COLOR.c_str();
	col[strlen(col) - 2] = '\0';

	char* tex = (char*)TEXCOORD.c_str();
	tex[strlen(tex) - 2] = '\0';

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ pos, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ col, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ tex, 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	HRESULT hr = S_OK;
	if (FAILED(hr = m_pDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_InputLayout)))
	{
	//	MessageBoxW(0, DXGetErrorDescription(hr), DXGetErrorString(hr), 0);
		SafeRelease(pVSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pVSBlob);

	if (FAILED(CompileShaderFromMemory(vertShader.c_str(), vertShader.length(), xorstr_("fontmain"), xorstr_("vs_4_0"), &pVSBlob)))
	{
		DestroyObjects();
		return;
	}

	if (FAILED(m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_fontvshader)))
	{
		DestroyObjects();
		pVSBlob->Release();
		return;
	}

	if (FAILED(m_pDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_fontInputLayout)))
	{
		SafeRelease(pVSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pVSBlob);

	ID3DBlob* pPSBlob = NULL;

	if (FAILED(CompileShaderFromMemory(pixShader.c_str(), pixShader.length(), xorstr_("main"), xorstr_("ps_4_0"), &pPSBlob)))
	{
		DestroyObjects();
		return;
	}

	// Create the pixel shader
	if (FAILED(m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pshader)))
	{
		SafeRelease(pPSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pPSBlob);

	if (FAILED(CompileShaderFromMemory(pixShader.c_str(), pixShader.length(), xorstr_("fontmain"), xorstr_("ps_4_0"), &pPSBlob)))
	{
		DestroyObjects();
		return;
	}

	// Create the pixel shader
	if (FAILED(m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_fontpshader)))
	{
		SafeRelease(pPSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pPSBlob);

	D3D11_BLEND_DESC blenddesc;
	blenddesc.AlphaToCoverageEnable = FALSE;
	blenddesc.IndependentBlendEnable = FALSE;
	blenddesc.RenderTarget[0].BlendEnable = TRUE;
	blenddesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blenddesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blenddesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blenddesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	if (FAILED(m_pDevice->CreateBlendState(&blenddesc, &m_blendstate)))
	{
		DestroyObjects();
		return;
	}


	D3D11_RASTERIZER_DESC rastdesc = CD3D11_RASTERIZER_DESC(D3D11_FILL_SOLID, D3D11_CULL_NONE, false, 0, 0.f, 0.f, false, false, false, false);

	if (FAILED(m_pDevice->CreateRasterizerState(&rastdesc, &m_raststate)))
	{
		DestroyObjects();
		return;
	}

	alive = true;

	D3D11_VIEWPORT vp;
	UINT numViewports = 1;
	m_pContext->RSGetViewports(&numViewports, &vp);

	width = vp.Width;
	height = vp.Height;

	alive = true;
}

void Renderer::SetDrawColor(Color color)
{
	m_Colour = ((int)color.w << 24) | ((int)color.x << 16) | ((int)color.y << 8) | (int)color.z;
}

high_resolution_clock::time_point lastTime = high_resolution_clock::time_point();

bool Renderer::CanBegin(const int fps)
{

	auto now = high_resolution_clock::now();
	auto millis = duration_cast<milliseconds>(now - lastTime).count();

	auto time = static_cast<int>(1.f / fps * std::milli::den);
	auto milliPerFrame = duration<long, std::milli>(time).count();
	if (millis >= milliPerFrame)
	{
		lastTime = now;
		return true;
	}
	return false;
}

bool Renderer::Begin(const int fps)
{
	if (!alive)
		return false;

	if (!CanBegin(fps))
		return false;

	if (FAILED(TriangleBuffer.Begin(m_pDevice)))
		return false;

	if (FAILED(LineBuffer.Begin(m_pDevice)))
	{
		TriangleBuffer.End();
		return false;
	}
	if (FAILED(FontBuffer.Begin(m_pDevice)))
	{
		TriangleBuffer.End();
		LineBuffer.End();
		return false;
	}

	for (auto & var : Fonts)
	{
		Fonts[var.first]->vertices.clear();
	}


	return true;
}

void Renderer::End()
{
	TriangleBuffer.End();
	LineBuffer.End();

	for (auto & var : Fonts)
	{
		FontBuffer.Add(var.second->vertices);
	}

	FontBuffer.End();
}

void Renderer::Present(IDXGISwapChain* pSwapChain)
{
	if (!alive)
		return;

	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11Texture2D* pBackBuffer;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);

	pBackBuffer->Release();

	m_pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);


	pRenderTargetView->Release();

	// Save current state
	/*m_pContext->OMGetBlendState(&m_pUILastBlendState, m_LastBlendFactor, &m_LastBlendMask);
	m_pContext->RSGetState(&m_pUILastRasterizerState);
	m_pContext->OMGetDepthStencilState(&m_LastDepthState, &m_LastStencilRef);
	m_pContext->IAGetInputLayout(&m_LastInputLayout);
	m_pContext->IAGetPrimitiveTopology(&m_LastTopology);
	m_pContext->IAGetVertexBuffers(0, 8, m_LastBuffers, m_LastStrides, m_LastOffsets);
	m_pContext->PSGetShader(&m_LastPSShader, NULL, 0);
	m_pContext->GSGetShader(&m_LastGSShader, NULL, 0);
	m_pContext->VSGetShader(&m_LastVSShader, NULL, 0);*/

	m_pContext->RSSetState(m_raststate);
	m_pContext->IASetInputLayout(m_InputLayout);

	m_pContext->OMSetBlendState(m_blendstate, NULL, 0xFFFFFFFF);



	m_pContext->PSSetShader(m_pshader, NULL, 0);
	m_pContext->VSSetShader(m_vshader, NULL, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	m_pContext->IASetVertexBuffers(0, 1, &LineBuffer.GetBuffer(), &stride, &offset);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_pContext->Draw(LineBuffer.GetNumVertices(), 0);

	stride = sizeof(Vertex);
	offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &TriangleBuffer.GetBuffer(), &stride, &offset);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->Draw(TriangleBuffer.GetNumVertices(), 0);

	stride = sizeof(FontVertex);
	offset = 0;
	m_pContext->IASetInputLayout(m_fontInputLayout);
	m_pContext->IASetVertexBuffers(0, 1, &FontBuffer.GetBuffer(), &stride, &offset);
	m_pContext->PSSetShader(m_fontpshader, NULL, 0);
	m_pContext->VSSetShader(m_fontvshader, NULL, 0);

	UINT currentIndex = 0;
	for (auto & var : Fonts)
	{
		auto& data = var.second;
		auto size = data->vertices.size();
		m_pContext->PSSetShaderResources(0, 1, &data->m_Texture);
		m_pContext->Draw(size, currentIndex);
		currentIndex += size;
	}

	/*m_pContext->OMSetBlendState(m_pUILastBlendState, m_LastBlendFactor, m_LastBlendMask);
	m_pContext->RSSetState(m_pUILastRasterizerState);
	m_pContext->OMSetDepthStencilState(m_LastDepthState, m_LastStencilRef);
	m_pContext->IASetInputLayout(m_LastInputLayout);
	m_pContext->IASetPrimitiveTopology(m_LastTopology);
	m_pContext->IASetVertexBuffers(0, 8, m_LastBuffers, m_LastStrides, m_LastOffsets);
	m_pContext->PSSetShader(m_LastPSShader, NULL, 0);
	m_pContext->GSSetShader(m_LastGSShader, NULL, 0);
	m_pContext->VSSetShader(m_LastVSShader, NULL, 0);*/
}


void Renderer::AddFilledRect(XMFLOAT4 rect)
{
	float scalex = 1 / width * 2.f;
	float scaley = 1 / height * 2.f;

	rect.z = (rect.x + rect.z) * scalex - 1.f;
	rect.w = 1.f - (rect.y + rect.w) * scaley;
	rect.x = rect.x * scalex - 1.f;
	rect.y = 1.f - rect.y * scaley;

	Vertex v[6] =
	{
		{ XMFLOAT3(rect.x, rect.w, 0.5f), m_Colour },
		{ XMFLOAT3(rect.x, rect.y, 0.5f), m_Colour },
		{ XMFLOAT3(rect.z, rect.w, 0.5f), m_Colour },
		{ XMFLOAT3(rect.z, rect.y, 0.5f), m_Colour },
		{ XMFLOAT3(rect.z, rect.w, 0.5f), m_Colour },
		{ XMFLOAT3(rect.x, rect.y, 0.5f), m_Colour }
	};
	TriangleBuffer.Add(v, ARRAYSIZE(v));
}

void Renderer::AddFilledLine(XMFLOAT4 rect)
{
	float scalex = 1 / width * 2.f;
	float scaley = 1 / height * 2.f;


	float xx0 = 2.0f * (rect.x - 0.5f) / width - 1.0f;
	float yy0 = 1.0f - 2.0f * (rect.y - 0.5f) / height;
	float xx1 = 2.0f * (rect.z - 0.5f) / width - 1.0f;
	float yy1 = 1.0f - 2.0f * (rect.w - 0.5f) / height;

	auto rec1 = XMFLOAT3(rect.x * scalex - 1.f, 1.f - rect.y * scaley, 1.f - (rect.y + 1) * scaley);
	auto rec2 = XMFLOAT3(rect.z * scalex - 1.f, 1.f - rect.w * scaley, 1.f - (rect.w + 1) * scaley);
	Vertex v[2] =
	{
		{ XMFLOAT3(xx0, yy0, 0.0f), m_Colour },
		{ XMFLOAT3(xx1, yy1, 0.0f), m_Colour },
	};
	LineBuffer.Add(v, ARRAYSIZE(v));
}

bool Renderer::containsFont(Font* font)
{
	return (Fonts.find(font) != Fonts.end());
}

HRESULT Renderer::LoadFont(Font* font)
{
	HRESULT hr = S_OK;
	if (containsFont(font))
		return S_OK;

	D3D11FontData* data(new D3D11FontData());
	std::unique_ptr<DWORD[]> bitmap;

	if (FAILED(hr = D3D11FontData::CreateFontObjects(*font, data, bitmap)))
		return hr;


	ID3D11Texture2D* buftex;
	D3D11_TEXTURE2D_DESC texdesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, data->m_TexWidth, data->m_TexHeight, 1, 1, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	if (FAILED(hr = m_pDevice->CreateTexture2D(&texdesc, nullptr, &buftex)))
	{
		return hr;
	}

	D3D11_MAPPED_SUBRESOURCE texmap;
	if (FAILED(hr = m_pContext->Map(buftex, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
	{
		SafeRelease(buftex);
		return hr;
	}

	BYTE bAlpha;
	DWORD* pDst32;
	BYTE* pDstRow = (BYTE*)texmap.pData;

	for (UINT32 y = 0; y < data->m_TexHeight; y++)
	{
		pDst32 = (DWORD*)pDstRow;
		for (UINT32 x = 0; x < data->m_TexWidth; x++)
		{
			bAlpha = BYTE((bitmap[data->m_TexWidth * y + x] & 0xFF) >> 4);
			if (bAlpha > 0)
				*pDst32++ = ((bAlpha * 0x11) << 24) | 0xFFFFFF;
			else
				*pDst32++ = 0x00000000;
		}
		pDstRow += texmap.RowPitch;
	}

	m_pContext->Unmap(buftex, 0);

	if (FAILED(hr = m_pDevice->CreateShaderResourceView(buftex, nullptr, &data->m_Texture)))
	{
		SafeRelease(buftex);
		return hr;
	}

	SafeRelease(buftex);

	Fonts.insert(std::pair<Font*, D3D11FontData*>(font, data));
	return S_OK;
}

void Renderer::FreeFont(Font* font)
{
	if (containsFont(font))
		return;

	auto data = Fonts[font];
	SafeRelease(data->m_Texture);

	Fonts.erase(font);

	delete data;
}


HRESULT Renderer::AddText(Font* font, float x, float y, const std::wstring& strText, DWORD dwFlag)
{
	return AddText(font, x, y, 1.f, strText, dwFlag);
}

HRESULT Renderer::AddText(Font* font, float x, float y, float scale, const std::wstring& strText, DWORD flag)
{
	HRESULT hr = S_OK;
	if (FAILED(hr = LoadFont(font)))
		return hr;

	float scalex = 1 / (float)width * 2.f;
	float scaley = 1 / (float)height * 2.f;

	XMFLOAT4A loc(x * scalex - 1.f, 1.f - y * scaley, 0.f, 0.f);

	auto& data = Fonts[font];

	if (flag != FONT_ALIGN_LEFT)
	{
		if (flag == FONT_ALIGN_RIGHT)
		{
			SIZE sz;
			GetTextExtent(data, strText, sz, scale);
			loc.x -= static_cast<float>(sz.cx) * scalex;
		}
		else if (flag == FONT_CENTERED)
		{
			SIZE sz;
			GetTextExtent(data, strText, sz, scale);
			loc.x -= static_cast<float>(sz.cx) / 2.0f * scalex;
		}
	}

	float fStartX = loc.x;

	for (auto c : strText)
	{
		if (c < 32 || c >= MAX_WCHAR)
		{
			if (c == '\n')
			{
				loc.x = fStartX;
				loc.y += (data->m_fTexCoords[c - 32].y - data->m_fTexCoords[c - 32].w) * data->m_TexHeight * scaley * scale;
			}
			else
				continue;
		}

		c -= 32;

		loc.z = loc.x + ((data->m_fTexCoords[c].z - data->m_fTexCoords[c].x) * data->m_TexWidth * scalex * scale);
		loc.w = loc.y + ((data->m_fTexCoords[c].y - data->m_fTexCoords[c].w) * data->m_TexHeight * scaley * scale);

		if (c != 0)
		{

			FontVertex v[6];
			v[0] = { { XMFLOAT3(loc.x, loc.w, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].x, data->m_fTexCoords[c].w) };
			v[1] = { { XMFLOAT3(loc.x, loc.y, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].x, data->m_fTexCoords[c].y) };
			v[2] = { { XMFLOAT3(loc.z, loc.w, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].z, data->m_fTexCoords[c].w) };
			v[3] = { { XMFLOAT3(loc.z, loc.y, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].z, data->m_fTexCoords[c].y) };
			v[4] = v[2];
			v[5] = v[1];

			for (auto & vertex : v)
			{
				data->vertices.push_back(vertex);
			}
		}
		loc.x += (loc.z - loc.x);
	}

	return S_OK;
}

HRESULT Renderer::GetTextExtent(FontData* font, const std::wstring& strText, SIZE& sz, float scale)
{
	float fRowWidth = 0.0f;
	float fRowHeight = (font->m_fTexCoords[0].y - font->m_fTexCoords[0].w) * font->m_TexHeight * scale;
	float fWidth = 0.0f;
	float fHeight = fRowHeight;
	const wchar_t max = (MAX_WCHAR - 32);

	for (auto c : strText)
	{
		if (c == '\n')
		{
			fRowWidth = 0.0f;
			fHeight += fRowHeight;
		}

		c -= 32;

		if (c < 0 || c >= max)
			continue;


		float tx1 = font->m_fTexCoords[c].x;
		float tx2 = font->m_fTexCoords[c].z;

		fRowWidth += (tx2 - tx1) * font->m_TexWidth;

		if (fRowWidth > fWidth)
			fWidth = fRowWidth;
	}
	sz.cx = static_cast<LONG>(fWidth * scale);
	sz.cy = static_cast<LONG>(fHeight * scale);

	return S_OK;
}

const std::wstring& Renderer::PrepareString(const wchar_t* str, ...)
{
	wchar_t wbuf[1024];

	va_list vlist;
	va_start(vlist, str);
	vswprintf(wbuf, str, vlist);
	va_end(vlist);

	std::wstring ret(wbuf);

	return ret;
}

const std::wstring& Renderer::PrepareString(const char* str, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, str);
	_vsnprintf(buf, sizeof(buf), str, va_alist);
	va_end(va_alist);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wsPath = converter.from_bytes(buf);
	return wsPath;
}

void Renderer::Text(int X, int Y, const std::wstring& txt, Color color, bool centered, Font* font)
{
	SetDrawColor(color);
	AddText(font, X, Y, txt, centered ? FONT_CENTERED : 0UL);
}


void Renderer::Text(int X, int Y, const std::string& txt, Color color, bool centered, Font* font)
{
	SetDrawColor(color);

	std::wstring wtxt = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(txt);

	AddText(font, X, Y, wtxt, centered ? FONT_CENTERED : 0UL);
}


void Renderer::Circle(int _x, int _y, int radius, Color color)
{
	const int NUMPOINTS = 30;

	Vertex v[NUMPOINTS + 1];

	float WedgeAngle = (float)((2 * XM_PI) / NUMPOINTS);

	SetDrawColor(color);

	for (int i = 0; i <= NUMPOINTS; i++)
	{
		float Theta = (float)(i * WedgeAngle);
		float x = (float)(_x + radius * cos(Theta));
		float y = (float)(_y - radius * sin(Theta));

		v[i].vec.x = 2.0f * (x - 0.5f) / width - 1.0f;
		v[i].vec.y = 1.0f - 2.0f * (y - 0.5f) / height;
		v[i].vec.z = 0.0f;
		v[i].color = m_Colour;
	}

	LineBuffer.Add(v, ARRAYSIZE(v));
}