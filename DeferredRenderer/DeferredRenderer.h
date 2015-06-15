#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

// ----------------------------------------------------------------------------

#include "SimpleMath.h"
#include "DXUtil.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

	void Initialize(ID3D11Device* pDevice);
	void Shutdown();
	void UpdateScene(const Matrix& wvp,
		ID3D11ShaderResourceView* pNormalTexture,
		ID3D11ShaderResourceView* pDiffuseTexture,
		ID3D11ShaderResourceView* pSpecularTexture,
		ID3D11ShaderResourceView* pPositionTexture,
		const Vector3& cameraPosition);
	void DrawScene(ID3D11DeviceContext* pDeviceContext,
		int iIndexCount, 
		Light* pointLightList);
	void SetShaderParameters(ID3D11DeviceContext* pDeviceContext);

private:

	// Methods

	void InitializeShaders(ID3D11Device* pDevice);
	void InitializeVertexLayout(ID3D11Device* pDevice);
	void InitializeSamplerState(ID3D11Device* pDevice);
	void InitializeMatrixBuffer(ID3D11Device* pDevice);
	void InitializeLightBuffer(ID3D11Device* pDevice);

	// Members

	ID3D11VertexShader*		m_pDeferredRendererVS;
	ID3D11PixelShader*		m_pDeferredRendererPS;

	ID3DBlob*				m_pDeferredRendererVS_Buffer;
	ID3DBlob*				m_pDeferredRendererPS_Buffer;

	ID3D11InputLayout*		m_pInputLayout;
	ID3D11SamplerState*		m_pSamplerState;
	ID3D11Buffer*			m_pMatrixBuffer;
	ID3D11Buffer*			m_pLightBuffer;

	// Structure type which will be sent to the vertex shader
	struct MatrixBuffer
	{
		Matrix WVP;
	};

	struct LightBuffer
	{
		Light light;
		Vector3 CameraPosition;
		float Pad;
	};

	// Temporary storage for the variables which need to be updated

	ID3D11ShaderResourceView* m_pNormalTexture;
	ID3D11ShaderResourceView* m_pDiffuseTexture;
	ID3D11ShaderResourceView* m_pSpecularTexture;
	ID3D11ShaderResourceView* m_pPositionTexture;

	MatrixBuffer				m_MatrixBuffer;
	LightBuffer					m_LightBuffer;
};

// ----------------------------------------------------------------------------


#endif // DEFERRED_RENDERER_H