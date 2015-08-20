#ifndef D3DAPPDEFERRED_H
#define D3DAPPDEFERRED_H

#include "BaseD3D.h"
#include "SimpleMath.h"

#include "Camera.h"

#include "DeferredBuffers.h"
#include "DeferredRenderer.h"
#include "DeferredRenderToTexture.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class D3DAppDeferred : public BaseD3D
{
public:
	D3DAppDeferred();
	virtual ~D3DAppDeferred();

	bool InitializeDirect3D11(HWND hwnd,
		HINSTANCE hInstance,
		unsigned int width,
		unsigned int height,
		bool fullScreen);
	void Shutdown();
	bool InitScene();
	void UpdateScene(double dt);
	void DrawScene(int iFPS, double dFrameTime);

	inline Camera* GetActiveCamera() { return m_pCamera.get(); }

	// Vertex structure
	struct Vertex
	{
		Vertex() {}
		Vertex(float x, float y, float z,
			float u, float v,
			float nx, float ny, float nz)
			: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}

		Vector3 pos;
		Vector2 texCoord;
		Vector3 normal;
	};

	struct VertexDeferred
	{
		VertexDeferred(float x, float y, float z, float u, float v)
			: pos(x, y, z), texCoord(u, v) {}

		Vector3 pos;
		Vector2 texCoord;
	};

private:

	void RenderToTextureSetup();
	void UpdateRTTCamera();
	void RenderToTexture(float color[4]);
	void RenderToBackBuffer(float color[4]);
	void EnableZBuffering();
	void DisableZBuffering();

	ID3D11DepthStencilState* m_pDepthStencilStateEnable;
	ID3D11DepthStencilState* m_pDepthStencilStateDisable;

	// Object mesh data
	ID3D11Buffer*			m_pVertexBuffer;
	ID3D11Buffer*			m_pIndexBuffer;

	// Ground mesh data
	ID3D11Buffer*			m_pGroundVertexBuffer;
	ID3D11Buffer*			m_pGroundIndexBuffer;

	// Texture quad data
	ID3D11Buffer*			m_pQuadVertexBuffer;
	ID3D11Buffer*			m_pQuadIndexBuffer;

	ID3D11VertexShader*		m_pVS;
	ID3D11PixelShader*		m_pPS;
	ID3D11PixelShader*		m_pQuadPS;
	ID3DBlob*				m_pVS_Buffer;
	ID3DBlob*				m_pPS_Buffer;
	ID3DBlob*				m_pQuadPS_Buffer;

	// Depth buffer
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D*		m_pDepthStencilBuffer;

	// Render state
	ID3D11RasterizerState*	m_pWireFrameRenderState;
	ID3D11RasterizerState*	m_pCCWCullMode;
	ID3D11RasterizerState*	m_pCWCullMode;

	// Texture
	ID3D11ShaderResourceView*	m_pCubeTexture;
	ID3D11ShaderResourceView*	m_pGroundTexture;

	// Original render target viewport
	D3D11_VIEWPORT m_Viewport;

	// ---------------------------------------------------------------------------
	// Deferred rendering

	DeferredBuffers*			m_pDeferredBuffers;
	DeferredRenderToTexture*	m_pDeferredRenderToTexture;
	DeferredRenderer*			m_pDeferredRenderer;

	// ---------------------------------------------------------------------------

	std::unique_ptr<Camera> m_pCamera;

	Matrix m_mWVP;

	Matrix m_mRotation;
	Matrix m_mGroundWorld;
	Matrix m_mScale;
	Matrix m_mTranslation;
	float m_fRotation = 0.01f;

	Light m_Light[MAXPOINTLIGHTS];

	// Material property
	Vector3 m_vSpecularAlbedo = Vector3(1.0f, 1.0f, 1.0f);
	float m_fSpecularPower = 2.0f;

	UINT m_uiStride = sizeof(Vertex);
	UINT m_uiStrideDeferredVertex = sizeof(VertexDeferred);
	UINT m_uiOffset = 0;
};

#endif // D3DAPPDEFERRED_H