#include "D3DApp.h"

#include <d3dcompiler.h>

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

UINT numElements = ARRAYSIZE(layout);

D3DApp::D3DApp()
{
}


D3DApp::~D3DApp()
{
}

bool D3DApp::InitializeDirect3D11(HWND hwnd, 
	HINSTANCE hInstance, 
	unsigned int width, 
	unsigned int height,
	bool fullScreen)
{
	// ---------------------------------------------------------------------------

	// Initialize client width and height
	m_iClientHeight		= height;
	m_iClientWidth		= width;

	// ---------------------------------------------------------------------------

	// Buffer description structure
	DXGI_MODE_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(DXGI_MODE_DESC));

	bufferDescription.Width						= width;
	bufferDescription.Height					= height;
	bufferDescription.RefreshRate.Numerator		= 60;
	bufferDescription.RefreshRate.Denominator	= 1;
	bufferDescription.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDescription.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDescription.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;

	// Swap chain description
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc			= bufferDescription;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.SampleDesc.Quality	= 0;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount			= 1;
	swapChainDesc.OutputWindow			= hwnd;
	swapChainDesc.Windowed				= fullScreen;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;

	// Create the swap chain, the device object and the device context
	HR(D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_pSwapChain,
		&m_pD3D11Device,
		NULL,
		&m_pD3D11DeviceContext));

	// ---------------------------------------------------------------------------

	// Create the back buffer
	ID3D11Texture2D* BackBuffer;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer));

	// Create the render target
	HR(m_pD3D11Device->CreateRenderTargetView(BackBuffer, NULL, &m_pRenderTargetView));
	BackBuffer->Release();

	// ---------------------------------------------------------------------------

	// Depth buffer description
	D3D11_TEXTURE2D_DESC depthStencilDescription;
	ZeroMemory(&depthStencilDescription, sizeof(D3D11_TEXTURE2D_DESC));
	depthStencilDescription.Width				= width;
	depthStencilDescription.Height				= height;
	depthStencilDescription.MipLevels			= 1;
	depthStencilDescription.ArraySize			= 1;
	depthStencilDescription.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT; // 24 bits for depth and 8 bits for stencil
	depthStencilDescription.SampleDesc.Count	= 1;
	depthStencilDescription.SampleDesc.Quality	= 0;
	depthStencilDescription.Usage				= D3D11_USAGE_DEFAULT;
	depthStencilDescription.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	depthStencilDescription.CPUAccessFlags		= 0;
	depthStencilDescription.MiscFlags			= 0;

	// Create the depth stencil texture
	m_pD3D11Device->CreateTexture2D(&depthStencilDescription, NULL, &m_pDepthStencilBuffer);

	// Create the depth stencil view
	m_pD3D11Device->CreateDepthStencilView(m_pDepthStencilBuffer, NULL, &m_pDepthStencilView);

	// Set the render target and depth target
	m_pD3D11DeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// ---------------------------------------------------------------------------

	// Initialize and create the render state
	D3D11_RASTERIZER_DESC rasterizerDescription;
	ZeroMemory(&rasterizerDescription, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDescription.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDescription.CullMode = D3D11_CULL_NONE;
	HR(m_pD3D11Device->CreateRasterizerState(&rasterizerDescription, &m_pWireFrameRenderState));

	// ---------------------------------------------------------------------------

	return true;
}

void D3DApp::ReleaseObjects()
{
	// D3D11
	m_pSwapChain->Release();
	m_pD3D11Device->Release();
	m_pD3D11DeviceContext->Release();

	// Rendering
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pVS->Release();
	m_pPS->Release();
	m_pVS_Buffer->Release();
	m_pPS_Buffer->Release();
	m_pVertexLayout->Release();

	// Release depth/stencil buffer/view
	m_pDepthStencilView->Release();
	m_pDepthStencilBuffer->Release();

	m_cbPerObjectBuffer->Release();

	m_pWireFrameRenderState->Release();
}

bool D3DApp::InitScene()
{
	// ---------------------------------------------------------------------------

	// Compile shaders from files
	HR(CompileShader(L"BasicVertexShader.hlsl", "main", "vs_5_0", &m_pVS_Buffer));
	HR(CompileShader(L"BasicPixelShader.hlsl", "main", "ps_5_0", &m_pPS_Buffer));

	// Create the shader objects
	HR(m_pD3D11Device->CreateVertexShader(m_pVS_Buffer->GetBufferPointer(), m_pVS_Buffer->GetBufferSize(), NULL, &m_pVS));
	HR(m_pD3D11Device->CreatePixelShader(m_pPS_Buffer->GetBufferPointer(), m_pPS_Buffer->GetBufferSize(), NULL, &m_pPS));

	// Set the vertex and the pixel shader
	m_pD3D11DeviceContext->VSSetShader(m_pVS, 0, 0);
	m_pD3D11DeviceContext->PSSetShader(m_pPS, 0, 0);

	// ---------------------------------------------------------------------------

	// Create the vertex buffer
	Vertex v[] =
	{
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, +1.0f, +1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
	};

	// Create the index buffer
	DWORD indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	// ---------------------------------------------------------------------------

	// Vertex buffer description
	D3D11_BUFFER_DESC vertexBufferDescription;
	ZeroMemory(&vertexBufferDescription, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	vertexBufferDescription.ByteWidth		= sizeof(Vertex) * 8;
	vertexBufferDescription.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescription.CPUAccessFlags	= 0;
	vertexBufferDescription.MiscFlags		= 0;

	// Index buffer description
	D3D11_BUFFER_DESC indexBufferDescription;
	ZeroMemory(&indexBufferDescription, sizeof(D3D11_BUFFER_DESC));
	indexBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	indexBufferDescription.ByteWidth		= sizeof(DWORD) * 3 * 12;
	indexBufferDescription.BindFlags		= D3D11_BIND_INDEX_BUFFER;
	indexBufferDescription.CPUAccessFlags	= 0;
	indexBufferDescription.MiscFlags		= 0;

	// Set vertex buffer data
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexBufferData.pSysMem = v;

	// Set index buffer data
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexBufferData.pSysMem = indices;

	// Create vertex buffer
	m_pD3D11Device->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &m_pVertexBuffer);

	// Create index buffer
	m_pD3D11Device->CreateBuffer(&indexBufferDescription, &indexBufferData, &m_pIndexBuffer);

	// Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set the index buffer
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Create the input layout
	m_pD3D11Device->CreateInputLayout(layout, numElements, m_pVS_Buffer->GetBufferPointer(), m_pVS_Buffer->GetBufferSize(), &m_pVertexLayout);

	// Set the input layout
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout);

	// Set primitive topology
	m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ---------------------------------------------------------------------------
	// Create constant buffer

	D3D11_BUFFER_DESC constBufferDescription;
	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth		= sizeof(m_cbPerObject);
	constBufferDescription.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags	= 0;
	constBufferDescription.MiscFlags		= 0;

	HR(m_pD3D11Device->CreateBuffer(&constBufferDescription, NULL, &m_cbPerObjectBuffer));

	// ---------------------------------------------------------------------------

	// Camera setup
	m_vCameraPosition = Vector4(0.0f, 3.0f, -8.0f, 0.0f);
	m_vCameraTarget = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_vCameraUp = Vector4(0.0f, 1.0f, 0.0f, 0.0f);

	// Create the view matrix
	m_mView = XMMatrixLookAtLH(m_vCameraPosition, m_vCameraTarget, m_vCameraUp);
	// Create the projection matrix
	m_mProjection = XMMatrixPerspectiveLH(0.4f * 3.14f,
		(float)m_iClientWidth / m_iClientHeight,
		1.0f,
		1000.0f);

	// ---------------------------------------------------------------------------

	// Create the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)m_iClientWidth;
	viewport.Height = (FLOAT)m_iClientHeight;
	// Om stage needs to convert the depth values between 0 - 1
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Set the viewport
	m_pD3D11DeviceContext->RSSetViewports(1, &viewport);

	return true;
}

void D3DApp::UpdateScene()
{
	//Update the colors of our scene
	red += colormodr * 0.00005f;
	green += colormodg * 0.00002f;
	blue += colormodb * 0.00001f;

	if (red >= 1.0f || red <= 0.0f)
		colormodr *= -1;
	if (green >= 1.0f || green <= 0.0f)
		colormodg *= -1;
	if (blue >= 1.0f || blue <= 0.0f)
		colormodb *= -1;

	// Rotate the cubes
	m_fRotation += 0.0005f;
	if (m_fRotation > 6.26f)
	{
		m_fRotation = 0.0f;
	}

	// Reset cube1 world
	m_mCube1World = Matrix();
	// Define cube1 world matrix
	m_mRotation		= Matrix::CreateRotationY(m_fRotation);
	m_mTranslation	= Matrix::CreateTranslation(0.0f, 0.0f, 4.0f);
	m_mCube1World	= m_mTranslation * m_mRotation;

	// Reset cube2 world
	m_mCube2World = Matrix();
	// Define cube2 world matrix
	m_mRotation		= Matrix::CreateRotationY(-m_fRotation);
	m_mScale		= Matrix::CreateScale(1.3f, 1.3f, 1.3f);
	m_mCube2World	= m_mRotation * m_mScale;
}

void D3DApp::DrawScene()
{
	// Clear the back buffer
	float bgColor[4];
	bgColor[0] = red;
	bgColor[1] = green;
	bgColor[2] = blue;
	bgColor[3] = 1.0f;

	// ---------------------------------------------------------------------------

	// Clear the render target view
	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView, bgColor);
	// Clear the depth/stencil view
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
		1.0f, // Value to set in the depth buffer
		0); // Value to set the stencil buffer

	// ---------------------------------------------------------------------------

	// Cube 1

	// Set the world/view/projection matrix
	m_mWVP = m_mCube1World * m_mView * m_mProjection;
	m_mWVP.Transpose(m_cbPerObject.WVP);
	m_pD3D11DeviceContext->UpdateSubresource(m_cbPerObjectBuffer, 0, NULL, &m_cbPerObject, 0, 0);
	m_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_cbPerObjectBuffer);
	// Set the render state
	m_pD3D11DeviceContext->RSSetState(m_pWireFrameRenderState);
	// Render the cube1
	m_pD3D11DeviceContext->DrawIndexed(36, 0, 0);
	// ---------------------------------------------------------------------------

	// Cube 2

	// Set the world/view/projection matrix
	m_mWVP = m_mCube2World * m_mView * m_mProjection;
	m_mWVP.Transpose(m_cbPerObject.WVP);
	m_pD3D11DeviceContext->UpdateSubresource(m_cbPerObjectBuffer, 0, NULL, &m_cbPerObject, 0, 0);
	m_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_cbPerObjectBuffer);
	// Set the render state
	m_pD3D11DeviceContext->RSSetState(NULL);
	// Render the cube2
	m_pD3D11DeviceContext->DrawIndexed(36, 0, 0);

	// ---------------------------------------------------------------------------

	// Present the back buffer to the screen
	m_pSwapChain->Present(0, 0);

	// ---------------------------------------------------------------------------
}

HRESULT D3DApp::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if (!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}
