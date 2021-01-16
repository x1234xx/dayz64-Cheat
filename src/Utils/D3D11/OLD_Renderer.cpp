#include "../../Includes.h"
#include "Renderer.h"
#include "Shader.h"
#include "Font.h"
#include <math.h>
#include <chrono>

#define CIRCLE_NUMPOINTS 30
#define MAX_VERTEX_COUNT ( CIRCLE_NUMPOINTS + 1 )
struct COLOR_VERTEX
{
	Vector3 position;
	Color color;
};

std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::time_point();
bool Renderer::ShouldClearBuffer()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();

	auto time = static_cast<int>(1.f / m_iFps * std::milli::den);
	auto milliPerFrame = std::chrono::duration<long, std::milli>(time).count();
	if (millis >= milliPerFrame)
	{
		 lastTime = now;
		 return true;
	}
	return false;
}

DWORD Renderer::ColorDWORD(Color color)
{
	return 0xFFFFFFFF;//((int)color.w << 24) | ((int)color.x << 16) | ((int)color.y << 8) | (int)color.z;
}
bool Renderer::Initialize(IDXGISwapChain * pSwapChain)
{
	if (!pSwapChain)
		return false;

	pSwapChain->GetDevice(__uuidof(m_pDevice), reinterpret_cast<void**>(&m_pDevice));
	m_pDevice->GetImmediateContext(&m_pDeviceContext);
	
	ID3D10Blob *compiledFX = NULL, *errorMsgs = NULL;

	if (FAILED(D3DX11CompileFromMemory(shaderRaw, strlen(shaderRaw), "FillTechFx", NULL, NULL, "FillTech", "fx_5_0", NULL, NULL, NULL, &compiledFX, &errorMsgs, NULL)))
		return false;

	if (FAILED(D3DX11CreateEffectFromMemory(compiledFX->GetBufferPointer(), compiledFX->GetBufferSize(), 0, this->m_pDevice, &m_pEffect)))
		return false;

	compiledFX->Release();

	m_pTechnique = m_pEffect->GetTechniqueByName("FillTech");

	if (m_pTechnique == NULL)
		return false;

	D3D11_INPUT_ELEMENT_DESC lineRectLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;

	if (FAILED(m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc)))
		return false;

	if (FAILED(this->m_pDevice->CreateInputLayout(lineRectLayout, sizeof(lineRectLayout) / sizeof(lineRectLayout[0]), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout)))
		return false;

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = MAX_VERTEX_COUNT * sizeof(COLOR_VERTEX);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;

	if (FAILED(this->m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_pVertexBuffer)))
		return false;

	m_pSprite = new DSprite();

	if (!m_pSprite->Initialize(m_pDevice, m_pDeviceContext))
		return false;

	return true;
}

void Renderer::SetFPS(const int fps)
{
	m_iFps = fps;
}

bool Renderer::Begin()
{
	if (!ShouldClearBuffer())
		return false;

	if (m_pSprite)
		m_pSprite->ClearBuffer();

	Buffer.Begin(m_pDevice);
	return true;
}

void Renderer::End()
{
	Buffer.End();
}

void Renderer::PreRender(IDXGISwapChain* pSwapChain)
{
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11Texture2D* pBackBuffer;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);

	pBackBuffer->Release();

	m_pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);


	pRenderTargetView->Release();


	UINT viewportNumber = 1;


	D3D11_VIEWPORT vp;
	this->m_pDeviceContext->RSGetViewports(&viewportNumber, &vp);
	m_iWidth = vp.Width;
	m_iHeight = vp.Height;
}

float _blendFactor[4];
void Renderer::Render()
{
	if (m_pSprite) {
		m_pSprite->RenderBatch(m_pFont->GetFontSheetSRV());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		_blendFactor[0] = { 1.0f };
		m_pDeviceContext->OMSetBlendState(m_pSprite->GetBlendState(), _blendFactor, 0xFFFFFFFF);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &Buffer.GetBuffer(), &stride, &offset);
		m_pDeviceContext->IASetInputLayout(m_pInputLayout);
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		
		D3DX11_TECHNIQUE_DESC techDesc;


		if (FAILED(m_pTechnique->GetDesc(&techDesc)))
			return;

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pTechnique->GetPassByIndex(p)->Apply(0, this->m_pDeviceContext);

			this->m_pDeviceContext->Draw(Buffer.GetNumVertices(), 0);
		}
		m_pDeviceContext->OMSetBlendState(0, _blendFactor, 0xFFFFFFFF);
	}	
}

void Renderer::Line(int x0, int y0, int x1, int y1, Color color)
{
	float scalex = 1 / m_iWidth * 2.f;
	float scaley = 1 / m_iHeight * 2.f;

	auto rec1 = XMFLOAT3(x0 * scalex - 1.f, 1.f - y0 * scaley, 1.f - (y0 + 1) * scaley);
	auto rec2 = XMFLOAT3(x1 * scalex - 1.f, 1.f - y1 * scaley, 1.f - (y1 + 1) * scaley);
	Vertex v[6] =
	{
		{ Vector3(rec2.x, rec2.y, 0.5f), color },
		{ Vector3(rec1.x, rec1.z, 0.5f), color },
		{ Vector3(rec1.x, rec1.y, 0.5f), color },

		{ Vector3(rec2.x, rec2.y, 0.5f), color },
		{ Vector3(rec2.x, rec2.z, 0.5f), color },
		{ Vector3(rec1.x, rec1.y, 0.5f), color },
	};
	Buffer.Add(v, ARRAYSIZE(v));

/*	this->m_pDeviceContext->Unmap(m_pVertexBuffer, NULL);

	this->m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT Stride = sizeof(COLOR_VERTEX);
	UINT Offset = 0;

	this->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);

	this->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	D3DX11_TECHNIQUE_DESC techDesc;


	if (FAILED(m_pTechnique->GetDesc(&techDesc)))
		return;

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, this->m_pDeviceContext);

		this->m_pDeviceContext->Draw(2, 0);
	}*/
}

void Renderer::Circle(int _x, int _y, int radius, Color color)
{

	const int NUMPOINTS = CIRCLE_NUMPOINTS;

	Vertex v[NUMPOINTS + 1];

	float WedgeAngle = (float)((2 * XM_PI) / NUMPOINTS);

	for (int i = 0; i <= NUMPOINTS; i++)
	{
		float Theta = (float)(i * WedgeAngle);
		float x = (float)(_x + radius * cos(Theta));
		float y = (float)(_y - radius * sin(Theta));

		v[i].position.x = 2.0f * (x - 0.5f) / m_iWidth - 1.0f;
		v[i].position.y = 1.0f - 2.0f * (y - 0.5f) / m_iHeight;
		v[i].position.z = 0.0f;
		v[i].color = color;
	}

	Buffer.Add(v, ARRAYSIZE(v));

/*	this->m_pDeviceContext->Unmap(m_pVertexBuffer, NULL);

	this->m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT Stride = sizeof(COLOR_VERTEX);
	UINT Offset = 0;

	this->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);

	this->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	D3DX11_TECHNIQUE_DESC techDesc;

	if (FAILED(m_pTechnique->GetDesc(&techDesc)))
		return;

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, this->m_pDeviceContext);

		this->m_pDeviceContext->Draw(NUMPOINTS + 1, 0);
	}*/
}


void Renderer::SetFont(DFont * pFont)
{
	m_pFont = pFont;
}

DFont * Renderer::CreateFont(const wchar_t * FontName, float FontSize, WORD FontStyle, bool AntiAliased)
{
	auto f = new DFont();

	if(f->Initialize(m_pDevice, m_pDeviceContext, FontName, FontSize, FontStyle, AntiAliased))
		return f;
	
	return NULL;
}

std::wstring& Renderer::PrepareString(const wchar_t* str, ...)
{
	wchar_t wbuf[1024];

	va_list vlist;
	va_start(vlist, str);
	wvsprintf(wbuf, str, vlist);
	va_end(vlist);

	std::wstring ret(wbuf);

	return ret;
}

std::wstring& Renderer::PrepareString(const char* str, ...)
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

void Renderer::Text(int X, int Y, std::wstring& text, Color color)
{
	m_pSprite->DrawString(X, Y, text, color.x, color.y, color.z, color.w, m_pFont);
}

void Renderer::Text(int X, int Y, const char * txt, Color color)
{
	size_t num;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wsPath = converter.from_bytes(txt);

	m_pSprite->DrawString(X, Y, wsPath, color.x, color.y, color.z, color.w, m_pFont);
}