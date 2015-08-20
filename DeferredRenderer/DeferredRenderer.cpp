#include "DeferredRenderer.h"

// ----------------------------------------------------------------------------

DeferredRenderer::DeferredRenderer()
{
	m_pDeferredRendererVS = nullptr;
	m_pDeferredRendererPS = nullptr;

	m_pDeferredRendererVS_Buffer = nullptr;
	m_pDeferredRendererPS_Buffer = nullptr;

	m_pSamplerState = nullptr;
}


DeferredRenderer::~DeferredRenderer()
{
}

// ----------------------------------------------------------------------------

void DeferredRenderer::Shutdown()
{
	// Release the g buffer shaders
	if (m_pDeferredRendererVS != nullptr)
	{
		m_pDeferredRendererVS->Release();
		m_pDeferredRendererVS = nullptr;
	}
	if (m_pDeferredRendererPS != nullptr)
	{
		m_pDeferredRendererPS->Release();
		m_pDeferredRendererPS = nullptr;
	}

	if (m_pDeferredRendererVS_Buffer != nullptr)
	{
		m_pDeferredRendererVS_Buffer->Release();
		m_pDeferredRendererVS_Buffer = nullptr;
	}
	if (m_pDeferredRendererPS_Buffer != nullptr)
	{
		m_pDeferredRendererPS_Buffer->Release();
		m_pDeferredRendererPS_Buffer = nullptr;
	}

	// Release the vertex layout
	if (m_pInputLayout != nullptr)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}

	// Release the matrix buffer
	if (m_pMatrixBuffer != nullptr)
	{
		m_pMatrixBuffer->Release();
		m_pMatrixBuffer = nullptr;
	}

	// Release the light buffer
	if (m_pLightBuffer != nullptr)
	{
		m_pLightBuffer->Release();
		m_pLightBuffer = nullptr;
	}

	// Release the sampler state
	if (m_pSamplerState != nullptr)
	{
		m_pSamplerState->Release();
		m_pSamplerState = nullptr;
	}
}

// ----------------------------------------------------------------------------

void DeferredRenderer::Initialize(ID3D11Device* pDevice)
{
	InitializeShaders(pDevice);
	InitializeVertexLayout(pDevice);
	InitializeSamplerState(pDevice);
	InitializeMatrixBuffer(pDevice);
	InitializeLightBuffer(pDevice);
}

// ----------------------------------------------------------------------------

void DeferredRenderer::UpdateScene(const Matrix& wvp,
	ID3D11ShaderResourceView* pNormalTexture,
	ID3D11ShaderResourceView* pDiffuseTexture,
	ID3D11ShaderResourceView* pSpecularTexture,
	ID3D11ShaderResourceView* pPositionTexture,
	const Vector3& cameraPosition)
{
	// Update the matrix buffer
	wvp.Transpose(m_MatrixBuffer.WVP);
	
	// Update the light buffer 
	m_LightBuffer.CameraPosition	= cameraPosition;

	// Update the source textures
	m_pNormalTexture	= pNormalTexture;
	m_pDiffuseTexture	= pDiffuseTexture;
	m_pSpecularTexture	= pSpecularTexture;
	m_pPositionTexture	= pPositionTexture;
}

// ----------------------------------------------------------------------------

void DeferredRenderer::DrawScene(ID3D11DeviceContext* pDeviceContext,
	int iIndexCount,
	Light* pointLightList)
{
	// Set vertex shader
	pDeviceContext->VSSetShader(m_pDeferredRendererVS, 0, 0);

	// Set pixel shader
	pDeviceContext->PSSetShader(m_pDeferredRendererPS, 0, 0);

	// Set input layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set shader parameters
	SetShaderParameters(pDeviceContext);


	for (int iPointLightIndex = 0; iPointLightIndex < MAXPOINTLIGHTS; iPointLightIndex++)
	{
		m_LightBuffer.light[iPointLightIndex] = pointLightList[iPointLightIndex];
	}

	pDeviceContext->UpdateSubresource(m_pLightBuffer, 0, NULL, &m_LightBuffer, 0, 0);
	pDeviceContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);

	// Render
	pDeviceContext->DrawIndexed(iIndexCount, 0, 0);

	// Reset the references to shader resource views
	ID3D11ShaderResourceView* shaderResView = NULL;
	pDeviceContext->PSSetShaderResources(0, 1, &shaderResView);
	pDeviceContext->PSSetShaderResources(1, 1, &shaderResView);
	pDeviceContext->PSSetShaderResources(2, 1, &shaderResView);
	pDeviceContext->PSSetShaderResources(3, 1, &shaderResView);
}

// ----------------------------------------------------------------------------

void DeferredRenderer::SetShaderParameters(ID3D11DeviceContext* pDeviceContext)
{
	pDeviceContext->UpdateSubresource(m_pMatrixBuffer, 0, NULL, &m_MatrixBuffer, 0, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffer);
	
	// Set shader texture resources in the pixel shader.
	pDeviceContext->PSSetShaderResources(0, 1, &m_pNormalTexture);
	pDeviceContext->PSSetShaderResources(1, 1, &m_pDiffuseTexture);
	pDeviceContext->PSSetShaderResources(2, 1, &m_pSpecularTexture);
	pDeviceContext->PSSetShaderResources(3, 1, &m_pPositionTexture);

	pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
}

// ----------------------------------------------------------------------------

void DeferredRenderer::InitializeShaders(ID3D11Device* pDevice)
{
	HR(CompileShader(L"DeferredVertexShader.hlsl", "main", "vs_5_0", &m_pDeferredRendererVS_Buffer));
	HR(CompileShader(L"DeferredPixelShader.hlsl", "main", "ps_5_0", &m_pDeferredRendererPS_Buffer));

	HR(pDevice->CreateVertexShader(m_pDeferredRendererVS_Buffer->GetBufferPointer(), m_pDeferredRendererVS_Buffer->GetBufferSize(), NULL, &m_pDeferredRendererVS));
	HR(pDevice->CreatePixelShader(m_pDeferredRendererPS_Buffer->GetBufferPointer(), m_pDeferredRendererPS_Buffer->GetBufferSize(), NULL, &m_pDeferredRendererPS));
}

// ----------------------------------------------------------------------------

void DeferredRenderer::InitializeVertexLayout(ID3D11Device* pDevice)
{
	// Fill the layout description
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// Create the layout
	pDevice->CreateInputLayout(layout,
		numElements,
		m_pDeferredRendererVS_Buffer->GetBufferPointer(),
		m_pDeferredRendererVS_Buffer->GetBufferSize(),
		&m_pInputLayout);
}

// ----------------------------------------------------------------------------

void DeferredRenderer::InitializeSamplerState(ID3D11Device* pDevice)
{
	// Fill in the sampler description
	D3D11_SAMPLER_DESC samplerDescription; // Describes how the shader will render the texture
	ZeroMemory(&samplerDescription, sizeof(D3D11_SAMPLER_DESC));
	samplerDescription.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDescription.AddressU			= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressV			= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressW			= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	samplerDescription.MinLOD			= 0;
	samplerDescription.MaxLOD			= D3D11_FLOAT32_MAX;

	// Create the sampler
	HR(pDevice->CreateSamplerState(&samplerDescription, &m_pSamplerState));
}

// ----------------------------------------------------------------------------

void DeferredRenderer::InitializeMatrixBuffer(ID3D11Device* pDevice)
{
	// Fill the constant buffer description
	D3D11_BUFFER_DESC constBufferDescription;
	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth = sizeof(MatrixBuffer);
	constBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags = 0;
	constBufferDescription.MiscFlags = 0;

	// Create constant buffer
	HR(pDevice->CreateBuffer(&constBufferDescription, NULL, &m_pMatrixBuffer));
}

// ----------------------------------------------------------------------------

void DeferredRenderer::InitializeLightBuffer(ID3D11Device* pDevice)
{
	// Fill the constant buffer description
	D3D11_BUFFER_DESC constBufferDescription;
	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth = sizeof(m_LightBuffer);
	constBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags = 0;
	constBufferDescription.MiscFlags = 0;

	// Create constant buffer
	HR(pDevice->CreateBuffer(&constBufferDescription, NULL, &m_pLightBuffer));
}

// ----------------------------------------------------------------------------