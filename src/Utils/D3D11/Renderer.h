#pragma once

#include "../../Includes.h"
#include "../../Singleton.h"
#include <map>

#undef CreateFont

class DFont;
class DSprite;

#define MAX_WCHAR 1791

#define FONT_ALIGN_LEFT 0
#define FONT_CENTERED 1
#define FONT_ALIGN_RIGHT 2

#ifndef SafeRelease
#define SafeRelease(var) if(var) {var->Release(); var = NULL;}
#endif

struct Font
{
public:
	std::wstring FontName;
	DWORD Size;
	DWORD Flags;

	Font(std::wstring fontName, DWORD dwHeight, DWORD dwFlags = 0)
		: FontName(fontName), Size(dwHeight), Flags(dwFlags)
	{}
};

class FontData
{
public:
	DirectX::XMFLOAT4 m_fTexCoords[MAX_WCHAR + 1];

	UINT32   m_TexWidth;
	UINT32   m_TexHeight;
	float   m_Spacing;

	virtual ~FontData() {}

	static HRESULT CreateFontObjects(const Font font, FontData* data, std::unique_ptr<DWORD[]>& lpBitmapBits);
};


static Font font_default(L"Arial", 10, FW_NORMAL);


class Renderer : public Singleton<Renderer>
{
protected:
	template <typename T>
	class VertexBuffer
	{
	protected:
		T* data;
		ID3D11Buffer* m_vbuffer;
		ID3D11DeviceContext* m_pContext;
		DWORD maxVertices;
		DWORD numVertices;
		bool bufferResize;
		bool open;
	public:
		inline VertexBuffer(DWORD _maxVertices = 1)
			:bufferResize(true), numVertices(0), maxVertices(_maxVertices), open(false), m_vbuffer(nullptr), m_pContext(nullptr), data(nullptr)
		{}

		inline ~VertexBuffer()
		{
			if (open)
				End();
			SafeRelease(m_vbuffer);
		}

		inline DWORD GetMaxVertices() const { return maxVertices; }

		inline DWORD GetNumVertices(void) const { return numVertices; }
		inline void SetNumVertices(DWORD value) { maxVertices = value; }

		inline ID3D11Buffer*& GetBuffer() { return m_vbuffer; }

		inline bool isBufferResizing() const { return (bufferResize || GetMaxVertices() < GetNumVertices()); }


		inline HRESULT Begin(ID3D11Device* pDevice)
		{
			HRESULT	hr;
			if (bufferResize)
			{
				SafeRelease(m_vbuffer);

				D3D11_BUFFER_DESC bufdesc = CD3D11_BUFFER_DESC(maxVertices * sizeof(T), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

				if (FAILED(hr = pDevice->CreateBuffer(&bufdesc, NULL, &m_vbuffer)))
					return hr;

				bufferResize = false;
			}

			pDevice->GetImmediateContext(&m_pContext);

			D3D11_MAPPED_SUBRESOURCE map;
			if (FAILED(hr = m_pContext->Map(m_vbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
				return hr;

			data = (T*)map.pData;

			numVertices = 0;

			open = true;
			return S_OK;
		}

		inline void Add(const std::vector<T>& vertices)
		{
			auto size = vertices.size();
			if (open && GetMaxVertices() >= GetNumVertices() + size)
			{
				memcpy(data, vertices.data(), sizeof(T) * size);
				data += size;
			}
			numVertices += size;
		}

		inline void Add(const T& vertex)
		{
			if (open && GetMaxVertices() >= GetNumVertices() + 1)
				*data++ = vertex;

		}

		inline void Add(const T* vertices, int len)
		{
			if (open && GetMaxVertices() >= GetNumVertices() + len)
			{
				memcpy(data, vertices, len * sizeof(T));
				data += len;
			}
			numVertices += len;
		}

		inline HRESULT End()
		{
			open = false;

			m_pContext->Unmap(m_vbuffer, 0);

			data = nullptr;

			if (bufferResize = isBufferResizing())
			{
				maxVertices = numVertices;
			}

			return S_OK;
		}

	};
	struct Vertex
	{
		DirectX::XMFLOAT3 vec;
		DWORD color;
	};

	struct FontVertex
	{
		Vertex vertex;
		DirectX::XMFLOAT2 uv;
	};

	class D3D11FontData : protected FontData
	{
		friend class Renderer;
	protected:
		ID3D11ShaderResourceView* m_Texture;
		std::vector<FontVertex> vertices;
	public:
		D3D11FontData() : FontData(), m_Texture(nullptr) {}
	};

	// Rendering
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	ID3D11InputLayout* m_InputLayout;
	ID3D11InputLayout* m_fontInputLayout;
	ID3D11BlendState* m_blendstate;
	ID3D11RasterizerState* m_raststate;

	//Primitive Renderering
	float width, height;
	VertexBuffer<Vertex> TriangleBuffer;
	VertexBuffer<Vertex> LineBuffer;
	VertexBuffer<FontVertex> FontBuffer;
	ID3D11PixelShader* m_pshader;
	ID3D11VertexShader* m_vshader;
	ID3D11PixelShader* m_fontpshader;
	ID3D11VertexShader* m_fontvshader;

	//Restores
	ID3D11BlendState* m_pUILastBlendState;
	float                   m_LastBlendFactor[4];
	UINT				    m_LastBlendMask;
	UINT				    m_LastStencilRef;
	ID3D11InputLayout* m_LastInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY m_LastTopology;
	ID3D11Buffer* m_LastBuffers[8];
	UINT				    m_LastStrides[8];
	UINT				    m_LastOffsets[8];
	ID3D11PixelShader* m_LastPSShader;
	ID3D11VertexShader* m_LastVSShader;
	ID3D11GeometryShader* m_LastGSShader;
	ID3D11DepthStencilState* m_LastDepthState;
	ID3D11RasterizerState* m_pUILastRasterizerState;

	//Font
	std::map<Font*, D3D11FontData*>  Fonts;
	inline bool containsFont(Font* font);

	//Management
	bool alive;

	DWORD m_Colour;

	void DestroyObjects();
	bool CanBegin(const int fps);
public:
	void Init(IDXGISwapChain* pSwapChain);

	void SetDrawColor(Color color);
	bool Begin(const int fps);
	void End();
	void Present(IDXGISwapChain* pSwapChain);

	void AddFilledRect(DirectX::XMFLOAT4 rect);
	void AddFilledLine(DirectX::XMFLOAT4 rect);

	HRESULT LoadFont(Font* font);
	void FreeFont(Font* font);

	HRESULT AddText(Font* font, float x, float y, const std::wstring& strText, DWORD dwFlag = 0);
	HRESULT AddText(Font* font, float x, float y, float scale, const std::wstring& strText, DWORD dwFlag = 0);
	HRESULT GetTextExtent(FontData* font, const std::wstring& strText, SIZE& sz, float scale = 1.f);

	void Text(int X, int Y, const std::wstring& txt, Color color, bool centered = false, Font* font=&font_default);
	void Text(int X, int Y, const std::string& txt, Color color, bool centered = false, Font* font = &font_default);
	const std::wstring& PrepareString(const wchar_t* str, ...);
	const std::wstring& PrepareString(const char* str, ...);
	void Circle(int x, int y, int radius, Color color);
};

