#ifndef DEFERRED_RENDERTOTEXTURE_H
#define DEFERRED_RENDERTOTEXTURE_H

// ----------------------------------------------------------------------------

#include "DXUtil.h"
#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ----------------------------------------------------------------------------

class DeferredRenderToTexture
{
public:
	DeferredRenderToTexture();
	~DeferredRenderToTexture();

	void Initialize(ID3D11Device* pDevice);
	void Shutdown();
	void UpdateScene(const Matrix& wvp,
		const Matrix& world,
		ID3D11ShaderResourceView* pTexture,
		const Vector3& specularAlbedo,
		float specularPower);
	void Draw(ID3D11DeviceContext* pDeviceContext,
		int iIndexCount);
	void SetShaderParameters(ID3D11DeviceContext* pDeviceContext);

private:

	// Methods

	void InitializeShaders(ID3D11Device* pDevice);
	void InitializeVertexLayout(ID3D11Device* pDevice);
	void InitializeSamplerState(ID3D11Device* pDevice);
	void InitializeConstantBuffer(ID3D11Device* pDevice);

	// Members

	ID3D11VertexShader*		m_pGBufferVS;
	ID3D11PixelShader*		m_pGBufferPS;

	ID3DBlob*				m_pGBufferVS_Buffer;
	ID3DBlob*				m_pGBufferPS_Buffer;

	ID3D11InputLayout*		m_pInputLayout;
	ID3D11SamplerState*		m_pSamplerState;
	ID3D11Buffer*			m_pMatrixConstantBuffer;
	ID3D11Buffer*			m_pSpecularConstantBuffer;

	ID3D11ShaderResourceView* m_pDiffuseAlbedoTexture;

	// Structure type which will be sent to the vertex shader
	struct MatrixBuffer
	{
		Matrix WVP;
		Matrix World;
	};

	MatrixBuffer m_MatrixBuffer;

	// Structure type which will be sent to the pixel shader
	struct SpecularDataBuffer
	{
		Vector3 SpecularAlbedo;
		float SpecularPower;
	};

	SpecularDataBuffer m_SpecularBuffer;
};

// ----------------------------------------------------------------------------

#endif // DEFERRED_RENDERTOTEXTURE_H