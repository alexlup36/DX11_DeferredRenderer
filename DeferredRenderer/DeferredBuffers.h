#ifndef DEFERREDBUFFERS_H
#define DEFERREDBUFFERS_H

#include <d3d11.h>

// Render target type
enum BUFFERTYPE
{
	keNormal,
	keDiffuseAlbedo,
	keSpecularAlbedo,
	kePosition,

	GBUFFER_COUNT,
};

class DeferredBuffers
{
public:
	DeferredBuffers();
	~DeferredBuffers();

	bool Initialize(ID3D11Device* device, 
		int iTextureWidth, 
		int iTextureHeight, 
		float fMaxDepth, 
		float fMinDepth,
		int iMsaaCount,
		int iMsaaQuality);
	void Shutdown();

	void SetRenderTargets(ID3D11DeviceContext* deviceContext);
	void ClearRenderTargets(ID3D11DeviceContext* deviceContext, float color[4]);

	inline ID3D11ShaderResourceView* GetShaderResourceView(int iRenderTargetIndex) 
	{ 
		if (iRenderTargetIndex < BUFFERTYPE::GBUFFER_COUNT)
		{
			return m_pShaderResourceViews[iRenderTargetIndex];
		}
		else
		{
			return nullptr;
		}
	}

private:
	int m_iTextureWidth;
	int m_iTextureHeight;

	ID3D11Texture2D*			m_pRenderTargetTextures[BUFFERTYPE::GBUFFER_COUNT];
	ID3D11RenderTargetView*		m_pRenderTagetViews[BUFFERTYPE::GBUFFER_COUNT];
	ID3D11ShaderResourceView*	m_pShaderResourceViews[BUFFERTYPE::GBUFFER_COUNT];
	ID3D11Texture2D*			m_pDepthStencilBuffer;
	ID3D11DepthStencilView*		m_pDepthStencilView;

	D3D11_VIEWPORT m_Viewport;
};

#endif // DEFERREDBUFFERS_H