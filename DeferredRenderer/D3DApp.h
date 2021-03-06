#ifndef D3DAPP_H
#define D3DAPP_H

#include "BaseD3D.h"
#include "SimpleMath.h"

#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class D3DApp : public BaseD3D
{
public:
	D3DApp();
	virtual ~D3DApp();

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

private:

	void Draw(const Matrix& view, const Matrix& projection, float color[4]);
	void RenderModels(const Matrix& viewProjection);

	// Object mesh data
	ID3D11Buffer*			m_pVertexBuffer;
	ID3D11Buffer*			m_pIndexBuffer;

	// Ground mesh data
	ID3D11Buffer*			m_pGroundVertexBuffer;
	ID3D11Buffer*			m_pGroundIndexBuffer;

	ID3D11VertexShader*		m_pVS;
	ID3D11PixelShader*		m_pPS;
	ID3DBlob*				m_pVS_Buffer;
	ID3DBlob*				m_pPS_Buffer;

	ID3D11InputLayout*		m_pVertexLayout;

	// Depth buffer
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D*		m_pDepthStencilBuffer;

	// Constant buffer per object
	ID3D11Buffer*			m_cbPerObjectBuffer;
	// Constant buffer per frame
	ID3D11Buffer*			m_cbPerFrameBuffer;

	// Render state
	ID3D11RasterizerState*	m_pWireFrameRenderState;
	ID3D11RasterizerState*	m_pCCWCullMode;
	ID3D11RasterizerState*	m_pCWCullMode;

	// Texture
	ID3D11ShaderResourceView*	m_pCubeTexture;
	ID3D11ShaderResourceView*	m_pGroundTexture;
	ID3D11SamplerState*			m_pCubeTexSameplerState;

	std::unique_ptr<Camera> m_pCamera;

	Matrix m_mWVP;

	Matrix m_mCube1World;
	Matrix m_mCube2World;
	Matrix m_mRotation;
	Matrix m_mGroundWorld;
	Matrix m_mScale;
	Matrix m_mTranslation;
	float m_fRotation = 0.01f;

	struct Light
	{
		Light()
		{
			ZeroMemory(this, sizeof(Light));
		}

		Vector3 Direction;
		float Pad1;

		Vector3 Position;
		float Range;
		
		Vector3 Attenuation;
		float Pad2;
		
		Vector3 Ambient;
		float Pad3;
		
		Vector3 Diffuse;
		float Pad4;
		
		Vector3 Specular;
		float Pad5;
		
		Vector2 SpotlightAngles;
		Vector2 Pad6;
	};
	Light m_Light;

	// Structure which will be sent to the vertex shader
	// Needs to have the exact same structure as the one in the vertex shader
	struct cbPerObject
	{
		Matrix WVP;
		Matrix World;
	};
	cbPerObject m_cbPerObjectStruct;
	
	// Structure which will be sent to the pixel shader
	// Needs to have the exact same structure as the one in the pixel shader
	struct cbPerFrame
	{
		Light light;
	};
	cbPerFrame m_cbPerFrameStruct;

	UINT m_uiStride = sizeof(Vertex);
	UINT m_uiOffset = 0;
};

#endif // D3DAPP_H