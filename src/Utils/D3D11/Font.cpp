#include "Font.h"

#pragma comment (lib,"Gdiplus.lib")
	// ******
	// DFont
	// ******

	DFont::DFont() : Initialized(false), FontSheetTex(0), FontSheetSRV(0), TexWidth(1024), TexHeight(0), SpaceWidth(0), CharHeight(0)
	{

	}
	DFont::~DFont()
	{
		SAFE_RELEASE(FontSheetTex)
			SAFE_RELEASE(FontSheetSRV)
	}
	bool DFont::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring & FontName, float FontSize, WORD FontStyle, bool AntiAliased)
	{
		if (Initialized)
			return true;

		pDevice = device;
		pDeviceContext = deviceContext;

		ULONG_PTR token = 0;
		Gdiplus::GdiplusStartupInput  startupInput(0, TRUE, TRUE);
		Gdiplus::GdiplusStartupOutput startupOutput;

		Gdiplus::GdiplusStartup(&token, &startupInput, &startupOutput);

		{
			Gdiplus::Font font(FontName.c_str(), FontSize, FontStyle, Gdiplus::UnitPixel);

			Gdiplus::TextRenderingHint hint = AntiAliased ? Gdiplus::TextRenderingHintAntiAlias : Gdiplus::TextRenderingHintSystemDefault;

			int                    tempSize = static_cast<int>(FontSize * 2);
			Gdiplus::Bitmap        charBitmap(tempSize, tempSize, PixelFormat32bppARGB);
			Gdiplus::Graphics    charGraphics(&charBitmap);

			charGraphics.SetPageUnit(Gdiplus::UnitPixel);
			charGraphics.SetTextRenderingHint(hint);

			MeasureChars(font, charGraphics);

			Gdiplus::Bitmap        fontSheetBitmap(TexWidth, TexHeight, PixelFormat32bppARGB);
			Gdiplus::Graphics    fontSheetGraphics(&fontSheetBitmap);

			fontSheetGraphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
			fontSheetGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

			BuildFontSheetBitmap(font, charGraphics, charBitmap, fontSheetGraphics);

			if (!BuildFontSheetTexture(fontSheetBitmap))
			{
				Gdiplus::GdiplusShutdown(token);

				return false;
			}
		}

		Gdiplus::GdiplusShutdown(token);

		Initialized = true;

		return true;
	}
	ID3D11ShaderResourceView * DFont::GetFontSheetSRV()
	{
		assert(Initialized);

		return FontSheetSRV;
	}
	const CD3D11_RECT & DFont::GetCharRect(WCHAR c)
	{
		assert(Initialized);

		return CharRects[c - StartChar];
	}
	int DFont::GetSpaceWidth()
	{
		assert(Initialized);

		return SpaceWidth;
	}
	int DFont::GetCharHeight()
	{
		assert(Initialized);

		return CharHeight;
	}
	void DFont::MeasureChars(Gdiplus::Font & font, Gdiplus::Graphics & charGraphics)
	{
		WCHAR allChars[NumChars + 1];

		for (WCHAR i = 0; i < NumChars; ++i)
			allChars[i] = StartChar + i;

		allChars[NumChars] = 0;

		Gdiplus::RectF sizeRect;
		charGraphics.MeasureString(allChars, NumChars, &font, Gdiplus::PointF(0, 0), &sizeRect);
		CharHeight = static_cast<int>(sizeRect.Height + 0.5f);

		int numRows = static_cast<int>(sizeRect.Width / TexWidth) + 1;
		TexHeight = static_cast<int>(numRows * CharHeight) + 1;

		WCHAR charString[2] = { ' ', 0 };
		charGraphics.MeasureString(charString, 1, &font, Gdiplus::PointF(0, 0), &sizeRect);
		SpaceWidth = static_cast<int>(sizeRect.Width + 0.5f);
	}
	void DFont::BuildFontSheetBitmap(Gdiplus::Font & font, Gdiplus::Graphics & charGraphics, Gdiplus::Bitmap & charBitmap, Gdiplus::Graphics & fontSheetGraphics)
	{
		WCHAR                charString[2] = { ' ', 0 };
		Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
		UINT                fontSheetX = 0;
		UINT                fontSheetY = 0;

		for (UINT i = 0; i < NumChars; ++i)
		{
			charString[0] = static_cast<WCHAR>(StartChar + i);
			charGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));
			charGraphics.DrawString(charString, 1, &font, Gdiplus::PointF(0.0f, 0.0f), &whiteBrush);

			int minX = GetCharMinX(charBitmap);
			int maxX = GetCharMaxX(charBitmap);
			int charWidth = maxX - minX + 1;

			if (fontSheetX + charWidth >= TexWidth)
			{
				fontSheetX = 0;
				fontSheetY += static_cast<int>(CharHeight) + 1;
			}

			CharRects[i] = CD3D11_RECT(fontSheetX, fontSheetY, fontSheetX + charWidth, fontSheetY + CharHeight);

			fontSheetGraphics.DrawImage(&charBitmap, fontSheetX, fontSheetY, minX, 0, charWidth, CharHeight, Gdiplus::UnitPixel);

			fontSheetX += charWidth + 1;
		}
	}
	bool DFont::BuildFontSheetTexture(Gdiplus::Bitmap & fontSheetBitmap)
	{
		Gdiplus::BitmapData bmData;

		fontSheetBitmap.LockBits(&Gdiplus::Rect(0, 0, TexWidth, TexHeight), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmData);

		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = TexWidth;
		texDesc.Height = TexHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = bmData.Scan0;
		data.SysMemPitch = TexWidth * 4;
		data.SysMemSlicePitch = 0;

		if (FAILED(pDevice->CreateTexture2D(&texDesc, &data, &FontSheetTex)))
			return false;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		if (FAILED(pDevice->CreateShaderResourceView(FontSheetTex, &srvDesc, &FontSheetSRV)))
			return false;

		fontSheetBitmap.UnlockBits(&bmData);

		return true;
	}
	int DFont::GetCharMinX(Gdiplus::Bitmap & charBitmap)
	{
		int width = charBitmap.GetWidth();
		int height = charBitmap.GetHeight();

		for (int x = 0; x < width; ++x)
		{
			for (int y = 0; y < height; ++y)
			{
				Gdiplus::Color color;

				charBitmap.GetPixel(x, y, &color);
				if (color.GetAlpha() > 0)
					return x;
			}
		}

		return 0;
	}
	int DFont::GetCharMaxX(Gdiplus::Bitmap & charBitmap)
	{
		int width = charBitmap.GetWidth();
		int height = charBitmap.GetHeight();

		for (int x = width - 1; x >= 0; --x)
		{
			for (int y = 0; y < height; ++y)
			{
				Gdiplus::Color color;

				charBitmap.GetPixel(x, y, &color);
				if (color.GetAlpha() > 0)
					return x;
			}
		}

		return width - 1;
	}
	int DFont::GetEncoderClsid(const WCHAR * format, CLSID * pClsid)
	{
		UINT                        num = 0, size = 0;
		Gdiplus::ImageCodecInfo *    pImageCodecInfo = 0;

		Gdiplus::GetImageEncodersSize(&num, &size);

		if (!size)
			return -1;

		pImageCodecInfo = (Gdiplus::ImageCodecInfo *)malloc(size);

		if (!pImageCodecInfo)
			return -1;

		Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT e = 0; e < num; ++e)
		{
			if (!wcscmp(pImageCodecInfo[e].MimeType, format))
			{
				*pClsid = pImageCodecInfo[e].Clsid;
				free(pImageCodecInfo);

				return e;
			}
		}

		free(pImageCodecInfo);
		return -1;
	}

	// ********
	// DSprite
	// ********

	DSprite::DSprite() : Initialized(false), VB(0), IB(0), InputLayout(0), ScreenWidth(0.0f), ScreenHeight(0.0f), BatchTexSRV(0), TexWidth(0), TexHeight(0)
	{

	}
	DSprite::~DSprite()
	{
		SAFE_RELEASE(VB)
			SAFE_RELEASE(IB)
			SAFE_RELEASE(InputLayout)
	}
	bool DSprite::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		assert(!Initialized);

		pDevice = device;
		pDeviceContext = deviceContext;

		ID3D10Blob *    CompiledFX = 0, *ErrorMsgs = 0;
		ID3DX11Effect *    Effect;

		const char SpriteFX[] = "Texture2D SpriteTex;"
			"SamplerState samLinear {"
			"     Filter = MIN_MAG_MIP_LINEAR;"
			"     AddressU = WRAP;"
			"     AddressV = WRAP;"
			"};"
			"struct VertexIn {"
			"     float3 PosNdc : POSITION;"
			"     float2 Tex    : TEXCOORD;"
			"     float4 Color  : COLOR;"
			"};"
			"struct VertexOut {"
			"     float4 PosNdc : SV_POSITION;"
			"     float2 Tex    : TEXCOORD;"
			"     float4 Color  : COLOR;"
			"};"
			"VertexOut VS(VertexIn vin) {"
			"     VertexOut vout;"
			"     vout.PosNdc = float4(vin.PosNdc, 1.0f);"
			"     vout.Tex    = vin.Tex;"
			"     vout.Color  = vin.Color;"
			"     return vout;"
			"};"
			"float4 PS(VertexOut pin) : SV_Target {"
			"     return pin.Color*SpriteTex.Sample(samLinear, pin.Tex);"
			"};"
			"technique11 SpriteTech {"
			"     pass P0 {"
			"         SetVertexShader( CompileShader( vs_5_0, VS() ) );"
			"         SetHullShader( NULL );"
			"         SetDomainShader( NULL );"
			"         SetGeometryShader( NULL );"
			"         SetPixelShader( CompileShader( ps_5_0, PS() ) );"
			"     }"
			"}";

		if (FAILED(D3DX11CompileFromMemory(SpriteFX, strlen(SpriteFX), 0, 0, 0, "SpriteTech", "fx_5_0", 0, 0, 0, &CompiledFX, &ErrorMsgs, 0)))
			return false;

		if (FAILED(D3DX11CreateEffectFromMemory(CompiledFX->GetBufferPointer(), CompiledFX->GetBufferSize(), 0, pDevice, &Effect)))
			return false;

		CompiledFX->Release();

		SpriteTech = Effect->GetTechniqueByName("SpriteTech");
		SpriteMap = Effect->GetVariableByName("SpriteTex")->AsShaderResource();

		SpriteList.reserve(128);

		std::vector<WORD> indices(3072);

		for (UINT i = 0; i < 512; ++i)
		{
			indices[i * 6] = i * 4;
			indices[i * 6 + 1] = i * 4 + 1;
			indices[i * 6 + 2] = i * 4 + 2;
			indices[i * 6 + 3] = i * 4;
			indices[i * 6 + 4] = i * 4 + 2;
			indices[i * 6 + 5] = i * 4 + 3;
		}

		D3D11_SUBRESOURCE_DATA indexData = { 0 };
		indexData.pSysMem = &indices[0];

		D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		D3DX11_PASS_DESC passDesc;

		SpriteTech->GetPassByIndex(0)->GetDesc(&passDesc);

		if (FAILED(pDevice->CreateInputLayout(layoutDesc, sizeof(layoutDesc) / sizeof(layoutDesc[0]), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &InputLayout)))
			return false;

		D3D11_BUFFER_DESC vbd;
		vbd.ByteWidth = 2048 * sizeof(SpriteVertex);
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		if (FAILED(pDevice->CreateBuffer(&vbd, 0, &VB)))
			return false;

		D3D11_BUFFER_DESC ibd;
		ibd.ByteWidth = 3072 * sizeof(WORD);
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		if (FAILED(pDevice->CreateBuffer(&ibd, &indexData, &IB)))
			return false;

		D3D11_BLEND_DESC transparentDesc = { 0 };
		transparentDesc.AlphaToCoverageEnable = false;
		transparentDesc.IndependentBlendEnable = false;
		transparentDesc.RenderTarget[0].BlendEnable = true;
		transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		pDevice->CreateBlendState(&transparentDesc, &TransparentBS);

		Initialized = true;

		return true;
	}
	void DSprite::DrawString(int X, int Y, const std::wstring & text, int R, int G, int B, int A, DFont* F)
	{
		float blendFactor[4] = { 1.0f };
		pDeviceContext->OMSetBlendState(TransparentBS, blendFactor, 0xFFFFFFFF);

	//	BeginBatch(F->GetFontSheetSRV());

		UINT length = text.length();

		int posX = X;
		int posY = Y;

		XMCOLOR color;
		XMVECTOR Vec = XMVectorSet(R ? (float)(R / 255.0f) : 0.0f, G ? (float)(G / 255.0f) : 0.0f, B ? (float)(B / 255.0f) : 0.0f, A ? (float)(A / 255.0f) : 0.0f);
		XMStoreColor(&color, Vec);

		for (UINT i = 0; i < length; ++i)
		{
			WCHAR character = text[i];

			if (character == ' ')
				posX += F->GetSpaceWidth();
			else if (character == '\n')
			{
				posX = X;
				posY += F->GetCharHeight();
			}
			else
			{
				const CD3D11_RECT & charRect = F->GetCharRect(character);

				int width = charRect.right - charRect.left;
				int height = charRect.bottom - charRect.top;

				Draw(CD3D11_RECT(posX, posY, posX + width, posY + height), charRect, color);

				posX += width + 1;
			}
		}

	//	EndBatch();

	}
	ID3D11BlendState* DSprite::GetBlendState()
	{
		return TransparentBS;
	}
	float blendFactor[4];
	void DSprite::ClearBuffer()
	{
		SpriteList.clear();
	}
	void DSprite::RenderBatch(ID3D11ShaderResourceView* texSRV)
	{
		if (!Initialized)
			return;

		blendFactor[0] = { 1.0f };
		pDeviceContext->OMSetBlendState(TransparentBS, blendFactor, 0xFFFFFFFF);

		BatchTexSRV = texSRV;
		BatchTexSRV->AddRef();

		ID3D11Resource* resource = 0;
		BatchTexSRV->GetResource(&resource);
		ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(resource);

		D3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		SAFE_RELEASE(resource)

			TexWidth = texDesc.Width;
		TexHeight = texDesc.Height;

		UINT viewportCount = 1;
		D3D11_VIEWPORT vp;

		pDeviceContext->RSGetViewports(&viewportCount, &vp);

		ScreenWidth = vp.Width;
		ScreenHeight = vp.Height;

		UINT stride = sizeof(SpriteVertex);
		UINT offset = 0;
		pDeviceContext->IASetInputLayout(InputLayout);
		pDeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R16_UINT, 0);
		pDeviceContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		SpriteMap->SetResource(BatchTexSRV);

		ID3DX11EffectPass * pass = SpriteTech->GetPassByIndex(0);
		pass->Apply(0, pDeviceContext);

		UINT spritesToDraw = SpriteList.size();
		UINT startIndex = 0;

		while (spritesToDraw > 0)
		{
			if (spritesToDraw <= 512)
			{
				DrawBatch(startIndex, spritesToDraw);
				spritesToDraw = 0;
			}
			else
			{
				DrawBatch(startIndex, 512);
				startIndex += 512;
				spritesToDraw -= 512;
			}
		}

		SAFE_RELEASE(BatchTexSRV)
		pDeviceContext->OMSetBlendState(0, blendFactor, 0xFFFFFFFF);
	}
	void DSprite::Draw(const CD3D11_RECT & destinationRect, const CD3D11_RECT & sourceRect, XMCOLOR color)
	{
		Sprite sprite;
		sprite.SrcRect = sourceRect;
		sprite.DestRect = destinationRect;
		sprite.Color = color;

		SpriteList.push_back(sprite);	
	}
	void DSprite::DrawBatch(UINT startSpriteIndex, UINT spriteCount)
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		pDeviceContext->Map(VB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		SpriteVertex * v = reinterpret_cast<SpriteVertex *>(mappedData.pData);

		for (UINT i = 0; i < spriteCount; ++i)
		{
			const Sprite & sprite = SpriteList[startSpriteIndex + i];

			SpriteVertex quad[4];
			BuildSpriteQuad(sprite, quad);

			v[i * 4] = quad[0];
			v[i * 4 + 1] = quad[1];
			v[i * 4 + 2] = quad[2];
			v[i * 4 + 3] = quad[3];
		}

		pDeviceContext->Unmap(VB, 0);

		pDeviceContext->DrawIndexed(spriteCount * 6, 0, 0);
	}
	XMFLOAT3 DSprite::PointToNdc(int x, int y, float z)
	{
		XMFLOAT3 p;

		p.x = 2.0f * (float)x / ScreenWidth - 1.0f;
		p.y = 1.0f - 2.0f * (float)y / ScreenHeight;
		p.z = z;

		return p;
	}
	void DSprite::BuildSpriteQuad(const Sprite & sprite, SpriteVertex v[4])
	{
		const CD3D11_RECT & dest = sprite.DestRect;
		const CD3D11_RECT & src = sprite.SrcRect;

		v[0].Pos = PointToNdc(dest.left, dest.bottom, sprite.Z);
		v[1].Pos = PointToNdc(dest.left, dest.top, sprite.Z);
		v[2].Pos = PointToNdc(dest.right, dest.top, sprite.Z);
		v[3].Pos = PointToNdc(dest.right, dest.bottom, sprite.Z);

		v[0].Tex = XMFLOAT2((float)src.left / TexWidth, (float)src.bottom / TexHeight);
		v[1].Tex = XMFLOAT2((float)src.left / TexWidth, (float)src.top / TexHeight);
		v[2].Tex = XMFLOAT2((float)src.right / TexWidth, (float)src.top / TexHeight);
		v[3].Tex = XMFLOAT2((float)src.right / TexWidth, (float)src.bottom / TexHeight);

		v[0].Color = sprite.Color;
		v[1].Color = sprite.Color;
		v[2].Color = sprite.Color;
		v[3].Color = sprite.Color;

		float tx = 0.5f * (v[0].Pos.x + v[3].Pos.x);
		float ty = 0.5f * (v[0].Pos.y + v[1].Pos.y);

		XMVECTOR scaling = XMVectorSet(sprite.Scale, sprite.Scale, 1.0f, 0.0f);
		XMVECTOR origin = XMVectorSet(tx, ty, 0.0f, 0.0f);
		XMVECTOR translation = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMMATRIX T = XMMatrixAffineTransformation2D(scaling, origin, sprite.Angle, translation);

		for (int i = 0; i < 4; ++i)
		{
			XMVECTOR p = XMLoadFloat3(&v[i].Pos);
			p = XMVector3TransformCoord(p, T);
			XMStoreFloat3(&v[i].Pos, p);
		}
	}