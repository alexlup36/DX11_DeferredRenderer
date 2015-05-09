#ifndef D3DAPP_H
#define D3DAPP_H

#include <windows.h>
#include <d3d11.h>

#include "DXUtil.h"
#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// Input layout description
extern D3D11_INPUT_ELEMENT_DESC layout[];
extern UINT numElements;

class D3DApp
{
public:
	D3DApp();
	virtual ~D3DApp();

	bool InitializeDirect3D11(HWND hwnd, 
		HINSTANCE hInstance, 
		unsigned int width, 
		unsigned int height,
		bool fullScreen);
	void ReleaseObjects();
	bool InitScene();
	void UpdateScene();
	void DrawScene();

	// Vertex structure
	struct Vertex
	{
		Vertex() {}
		Vertex(float x, float y, float z,
			float cr, float cg, float cb, float ca)
			: pos(x, y, z), color(cr, cg, cb, ca) {}

		Vector3 pos;
		Vector4 color;
	};

private:
	HRESULT CompileShader(_In_ LPCWSTR srcFile,
		_In_ LPCSTR entryPoint,
		_In_ LPCSTR profile,
		_Outptr_ ID3DBlob** blob);

	IDXGISwapChain*			m_pSwapChain;
	ID3D11Device*			m_pD3D11Device;
	ID3D11DeviceContext*	m_pD3D11DeviceContext;
	ID3D11RenderTargetView* m_pRenderTargetView;

	ID3D11Buffer*			m_pVertexBuffer;
	ID3D11Buffer*			m_pIndexBuffer;
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

	// Render state
	ID3D11RasterizerState*	m_pWireFrameRenderState;

	Matrix m_mWVP;
	Matrix m_mWorld;
	Matrix m_mView;
	Matrix m_mProjection;
	Vector4 m_vCameraPosition;
	Vector4 m_vCameraTarget;
	Vector4 m_vCameraUp;

	Matrix m_mCube1World;
	Matrix m_mCube2World;
	Matrix m_mRotation;
	Matrix m_mScale;
	Matrix m_mTranslation;
	float m_fRotation = 0.01f;

	// Structure which will be sent to the vertex shader
	// Needs to have the exact same structure as the one in the vertex shader
	struct cbPerObject
	{
		Matrix WVP;
	};
	cbPerObject m_cbPerObject;

	int m_iClientWidth;
	int m_iClientHeight;

	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	int colormodr = 1;
	int colormodg = 1;
	int colormodb = 1;
};

#endif // D3DAPP_H