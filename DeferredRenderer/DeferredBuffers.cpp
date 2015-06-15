#include "DeferredBuffers.h"

#include "DXUtil.h"

// ----------------------------------------------------------------------------

DeferredBuffers::DeferredBuffers()
{
	for (int iIndex = 0; iIndex < BUFFERTYPE::GBUFFER_COUNT; iIndex++)
	{
		m_pRenderTargetTextures[iIndex]		= nullptr;
		m_pRenderTagetViews[iIndex]			= nullptr;
		m_pShaderResourceViews[iIndex]		= nullptr;
	}

	m_pDepthStencilBuffer	= nullptr;
	m_pDepthStencilView		= nullptr;
}

// ----------------------------------------------------------------------------

DeferredBuffers::~DeferredBuffers()
{
}

// ----------------------------------------------------------------------------

bool DeferredBuffers::Initialize(ID3D11Device* device, 
	int iTextureWidth, 
	int iTextureHeight, 
	float fMaxDepth, 
	float fMinDepth,
	int iMsaaCount,
	int iMsaaQuality)
{
	// Initialize the texture size
	m_iTextureHeight = iTextureHeight;
	m_iTextureWidth = iTextureWidth;

	// ----------------------------------------------------------------------------
	// Create textures for all render targets
	D3D11_TEXTURE2D_DESC textureDescription;
	ZeroMemory(&textureDescription, sizeof(D3D11_TEXTURE2D_DESC));
	// Fill the texture description
	textureDescription.Width				= m_iTextureWidth;
	textureDescription.Height				= m_iTextureHeight;
	textureDescription.MipLevels			= 1;
	textureDescription.ArraySize			= 1;
	textureDescription.Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDescription.SampleDesc.Count		= 1;
	textureDescription.SampleDesc.Quality	= 0;
	textureDescription.Usage				= D3D11_USAGE_DEFAULT;
	textureDescription.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDescription.CPUAccessFlags		= 0;
	textureDescription.MiscFlags			= 0;

	// Create textures
	for (int iTextureIndex = 0; iTextureIndex < BUFFERTYPE::GBUFFER_COUNT; iTextureIndex++)
	{
		HR(device->CreateTexture2D(&textureDescription, NULL, &m_pRenderTargetTextures[iTextureIndex]));
	}

	// ----------------------------------------------------------------------------
	// Create render target view to be able to access the render target textures
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription;
	ZeroMemory(&renderTargetViewDescription, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	// Fill the render target view description
	renderTargetViewDescription.Format				= textureDescription.Format;
	renderTargetViewDescription.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDescription.Texture2D.MipSlice	= 0;

	// Create the render target views
	for (int iTextureIndex = 0; iTextureIndex < BUFFERTYPE::GBUFFER_COUNT; iTextureIndex++)
	{
		HR(device->CreateRenderTargetView(m_pRenderTargetTextures[iTextureIndex],
			&renderTargetViewDescription,
			&m_pRenderTagetViews[iTextureIndex]));
	}

	// ----------------------------------------------------------------------------
	// Create the shader resource views for each texture
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;
	ZeroMemory(&shaderResourceViewDescription, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	// Fill the shader resource view description
	shaderResourceViewDescription.Format						= textureDescription.Format;
	shaderResourceViewDescription.ViewDimension					= D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDescription.Texture2D.MostDetailedMip		= 0;
	shaderResourceViewDescription.Texture2D.MipLevels			= 1;

	// Create the shader resource views
	for (int iTextureIndex = 0; iTextureIndex < BUFFERTYPE::GBUFFER_COUNT; iTextureIndex++)
	{
		HR(device->CreateShaderResourceView(m_pRenderTargetTextures[iTextureIndex],
			&shaderResourceViewDescription,
			&m_pShaderResourceViews[iTextureIndex]));
	}

	// ----------------------------------------------------------------------------
	// Create the depth/stencil buffer and view

	// Fill the depth buffer texture description
	D3D11_TEXTURE2D_DESC depthBufferDescription;
	ZeroMemory(&depthBufferDescription, sizeof(D3D11_TEXTURE2D_DESC));
	depthBufferDescription.Width				= m_iTextureWidth;
	depthBufferDescription.Height				= m_iTextureHeight;
	depthBufferDescription.MipLevels			= 1;
	depthBufferDescription.ArraySize			= 1;
	depthBufferDescription.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDescription.SampleDesc.Count		= 1;
	depthBufferDescription.SampleDesc.Quality	= 0;
	depthBufferDescription.Usage				= D3D11_USAGE_DEFAULT;
	depthBufferDescription.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	depthBufferDescription.CPUAccessFlags		= 0;
	depthBufferDescription.MiscFlags			= 0;

	// Create the texture for the depth buffer
	HR(device->CreateTexture2D(&depthBufferDescription, NULL, &m_pDepthStencilBuffer));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription;
	ZeroMemory(&depthStencilViewDescription, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	// Fill the depth/stencil description
	depthStencilViewDescription.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDescription.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDescription.Texture2D.MipSlice	= 0;
	depthStencilViewDescription.Flags				= 0;

	// Create the depth/stencil view
	HR(device->CreateDepthStencilView(m_pDepthStencilBuffer, 
		&depthStencilViewDescription,
		&m_pDepthStencilView));

	// ----------------------------------------------------------------------------
	// Setup the viewport for rendering

	m_Viewport.Width		= (float)m_iTextureWidth;
	m_Viewport.Height		= (float)m_iTextureHeight;
	m_Viewport.MinDepth		= 0.0f;
	m_Viewport.MaxDepth		= 1.0f;
	m_Viewport.TopLeftX		= 0.0f;
	m_Viewport.TopLeftY		= 0.0f;

	// ------------------------------------------------------------------------

	// Everything has been successfully initialized
	return true;

	// ------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------

void DeferredBuffers::Shutdown()
{
	// Release the render targets
	for (int iIndex = 0; iIndex < BUFFERTYPE::GBUFFER_COUNT; iIndex++)
	{
		if (m_pRenderTargetTextures[iIndex] != nullptr)
		{
			m_pRenderTargetTextures[iIndex]->Release();
		}
		
		if (m_pRenderTagetViews[iIndex] != nullptr)
		{
			m_pRenderTagetViews[iIndex]->Release();
		}
		
		if (m_pShaderResourceViews[iIndex] != nullptr)
		{
			m_pShaderResourceViews[iIndex]->Release();
		}
	}

	// Release the depth/stencil target and view
	if (m_pDepthStencilBuffer != nullptr)
	{
		m_pDepthStencilBuffer->Release();
	}
	
	if (m_pDepthStencilView != nullptr)
	{
		m_pDepthStencilView->Release();
	}
}

// ----------------------------------------------------------------------------

void DeferredBuffers::SetRenderTargets(ID3D11DeviceContext* deviceContext)
{
	// Sets the render targets in the array as the location where the shaders will write to
	deviceContext->OMSetRenderTargets(BUFFERTYPE::GBUFFER_COUNT, m_pRenderTagetViews, m_pDepthStencilView);

	// Set the viewport
	deviceContext->RSSetViewports(1, &m_Viewport);
}

// ----------------------------------------------------------------------------

void DeferredBuffers::ClearRenderTargets(ID3D11DeviceContext* deviceContext, float color[4])
{
	// Clear all the render targets
	for (int iTextureIndex = 0; iTextureIndex < BUFFERTYPE::GBUFFER_COUNT; iTextureIndex++)
	{
		deviceContext->ClearRenderTargetView(m_pRenderTagetViews[iTextureIndex], color);
	}

	// Clear the depth/stencil buffer
	deviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

// ----------------------------------------------------------------------------