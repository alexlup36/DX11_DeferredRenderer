#include "DeferredRenderToTexture.h"

// ----------------------------------------------------------------------------

DeferredRenderToTexture::DeferredRenderToTexture()
{
	m_pGBufferVS = nullptr;
	m_pGBufferPS = nullptr;

	m_pGBufferVS_Buffer = nullptr;
	m_pGBufferPS_Buffer = nullptr;

	m_pMatrixConstantBuffer		= nullptr;
	m_pSpecularConstantBuffer	= nullptr;
}


DeferredRenderToTexture::~DeferredRenderToTexture()
{
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::Shutdown()
{
	// Release the g buffer shaders
	if (m_pGBufferVS != nullptr)
	{
		m_pGBufferVS->Release();
		m_pGBufferVS = nullptr;
	}
	if (m_pGBufferPS != nullptr)
	{
		m_pGBufferPS->Release();
		m_pGBufferPS = nullptr;
	}

	if (m_pGBufferVS_Buffer != nullptr)
	{
		m_pGBufferVS_Buffer->Release();
		m_pGBufferVS_Buffer = nullptr;
	}
	if (m_pGBufferPS_Buffer != nullptr)
	{
		m_pGBufferPS_Buffer->Release();
		m_pGBufferPS_Buffer = nullptr;
	}

	// Release the vertex layout
	if (m_pInputLayout != nullptr)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}

	// Release the sampler state
	if (m_pSamplerState != nullptr)
	{
		m_pSamplerState->Release();
		m_pSamplerState = nullptr;
	}

	// Release the matrix constant buffer
	if (m_pMatrixConstantBuffer != nullptr)
	{
		m_pMatrixConstantBuffer->Release();
		m_pMatrixConstantBuffer = nullptr;
	}

	// Release the specular constant buffer
	if (m_pSpecularConstantBuffer != nullptr)
	{
		m_pSpecularConstantBuffer->Release();
		m_pSpecularConstantBuffer = nullptr;
	}
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::Initialize(ID3D11Device* pDevice)
{
	InitializeShaders(pDevice);
	InitializeVertexLayout(pDevice);
	InitializeSamplerState(pDevice);
	InitializeConstantBuffer(pDevice);
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::UpdateScene(const Matrix& wvp,
	const Matrix& world,
	ID3D11ShaderResourceView* pTexture,
	const Vector3& specularAlbedo,
	float specularPower)
{
	// Update the matrix constant buffer
	world.Transpose(m_MatrixBuffer.World);
	wvp.Transpose(m_MatrixBuffer.WVP);

	// Update the diffuse albedo texture
	m_pDiffuseAlbedoTexture = pTexture;

	// Update the specular constant buffer
	m_SpecularBuffer.SpecularAlbedo = specularAlbedo;
	m_SpecularBuffer.SpecularPower = specularPower;
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::Draw(ID3D11DeviceContext* pDeviceContext,
	int iIndexCount)
{
	// Set vertex shader
	pDeviceContext->VSSetShader(m_pGBufferVS, 0, 0);

	// Set pixel shader
	pDeviceContext->PSSetShader(m_pGBufferPS, 0, 0);

	// Set shader parameters
	SetShaderParameters(pDeviceContext);

	// Set input layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);
	
	// Render
	pDeviceContext->DrawIndexed(iIndexCount, 0, 0);
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::SetShaderParameters(ID3D11DeviceContext* pDeviceContext)
{
	pDeviceContext->UpdateSubresource(m_pMatrixConstantBuffer, 0, NULL, &m_MatrixBuffer, 0, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &m_pMatrixConstantBuffer);

	pDeviceContext->UpdateSubresource(m_pSpecularConstantBuffer, 0, NULL, &m_SpecularBuffer, 0, 0);
	pDeviceContext->PSSetConstantBuffers(0, 1, &m_pSpecularConstantBuffer);

	pDeviceContext->PSSetShaderResources(0, 1, &m_pDiffuseAlbedoTexture);
	pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::InitializeShaders(ID3D11Device* pDevice)
{
	HR(CompileShader(L"GBufferVertexShader.hlsl", "main", "vs_5_0", &m_pGBufferVS_Buffer));
	HR(CompileShader(L"GBufferPixelShader.hlsl", "main", "ps_5_0", &m_pGBufferPS_Buffer));

	HR(pDevice->CreateVertexShader(m_pGBufferVS_Buffer->GetBufferPointer(), m_pGBufferVS_Buffer->GetBufferSize(), NULL, &m_pGBufferVS));
	HR(pDevice->CreatePixelShader(m_pGBufferPS_Buffer->GetBufferPointer(), m_pGBufferPS_Buffer->GetBufferSize(), NULL, &m_pGBufferPS));
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::InitializeVertexLayout(ID3D11Device* pDevice)
{
	// Fill the layout description
	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// Create the layout
	pDevice->CreateInputLayout(layout,
		numElements,
		m_pGBufferVS_Buffer->GetBufferPointer(),
		m_pGBufferVS_Buffer->GetBufferSize(),
		&m_pInputLayout);
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::InitializeSamplerState(ID3D11Device* pDevice)
{
	// Fill in the sampler description
	D3D11_SAMPLER_DESC samplerDescription; // Describes how the shader will render the texture
	ZeroMemory(&samplerDescription, sizeof(D3D11_SAMPLER_DESC));
	samplerDescription.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescription.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	samplerDescription.MinLOD			= 0;
	samplerDescription.MaxLOD			= D3D11_FLOAT32_MAX;

	// Create the sampler
	HR(pDevice->CreateSamplerState(&samplerDescription, &m_pSamplerState));
}

// ----------------------------------------------------------------------------

void DeferredRenderToTexture::InitializeConstantBuffer(ID3D11Device* pDevice)
{
	// ---------------------------------------------------------------------------

	// Matrix constant buffer
	// Fill the constant buffer description
	D3D11_BUFFER_DESC constBufferDescription;
	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth		= sizeof(MatrixBuffer);
	constBufferDescription.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags	= 0;
	constBufferDescription.MiscFlags		= 0;

	// Create constant buffer
	HR(pDevice->CreateBuffer(&constBufferDescription, NULL, &m_pMatrixConstantBuffer));

	// ---------------------------------------------------------------------------

	// Specular data constant buffer
	// Reset the buffer description
	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth		= sizeof(SpecularDataBuffer);
	constBufferDescription.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags	= 0;
	constBufferDescription.MiscFlags		= 0;

	// Create constant buffer
	HR(pDevice->CreateBuffer(&constBufferDescription, NULL, &m_pSpecularConstantBuffer));

	// ---------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------