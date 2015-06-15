#include "D3DApp.h"

#include <d3dcompiler.h>
#include <string>
#include "WICTextureLoader.h"

#include "TextRenderer.h"

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
	// Call base initialize
	BaseD3D::InitializeDirect3D11(hwnd, hInstance, width, height, fullScreen);

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

	// Enable multisampling
	if (m_bEnable4xMsaa)
	{
		depthStencilDescription.SampleDesc.Count = m_iMSAACount;
		depthStencilDescription.SampleDesc.Quality = m_iMSAAQuality - 1;
	}

	// Create the depth stencil texture
	m_pD3D11Device->CreateTexture2D(&depthStencilDescription, NULL, &m_pDepthStencilBuffer);

	// Create the depth stencil view
	m_pD3D11Device->CreateDepthStencilView(m_pDepthStencilBuffer, NULL, &m_pDepthStencilView);

	// Set the render target and depth target
	m_pD3D11DeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	m_pD3D11DeviceContext->OMSetDepthStencilState(NULL, 0);

	// ---------------------------------------------------------------------------

	// Initialize and create the render state
	D3D11_RASTERIZER_DESC rasterizerDescription;
	ZeroMemory(&rasterizerDescription, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDescription.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDescription.CullMode = D3D11_CULL_NONE;
	HR(m_pD3D11Device->CreateRasterizerState(&rasterizerDescription, &m_pWireFrameRenderState));

	// Create CW and CCW rasterization states
	ZeroMemory(&rasterizerDescription, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.CullMode = D3D11_CULL_BACK;
	rasterizerDescription.FrontCounterClockwise = true;
	HR(m_pD3D11Device->CreateRasterizerState(&rasterizerDescription, &m_pCCWCullMode));

	rasterizerDescription.FrontCounterClockwise = false;
	HR(m_pD3D11Device->CreateRasterizerState(&rasterizerDescription, &m_pCWCullMode));

	// ---------------------------------------------------------------------------

	return true;
}

void D3DApp::Shutdown()
{
	// ---------------------------------------------------------------------------
	
	BaseD3D::Shutdown();

	// ---------------------------------------------------------------------------

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

	// Constant buffers
	m_cbPerObjectBuffer->Release();
	m_cbPerFrameBuffer->Release();

	// Render state
	m_pWireFrameRenderState->Release();
	m_pCCWCullMode->Release();
	m_pCWCullMode->Release();
}

bool D3DApp::InitScene()
{
	// ---------------------------------------------------------------------------
	// Initialize frustum near and far planes

	m_fScreenDepth = 500.0f;
	m_fScreenNear = 1.0f;

	// ---------------------------------------------------------------------------

	// Create camera instance
	m_pCamera = std::make_unique<Camera>(m_fNearPlaneWidth, 
		m_fScreenNear, 
		m_fScreenDepth, 
		m_iClientWidth, 
		m_iClientHeight);

	// ---------------------------------------------------------------------------

	// Compile shaders from files and create shader object
	HR(CompileShader(L"BasicVertexShader.hlsl", "main", "vs_5_0", &m_pVS_Buffer));
	HR(m_pD3D11Device->CreateVertexShader(m_pVS_Buffer->GetBufferPointer(), m_pVS_Buffer->GetBufferSize(), NULL, &m_pVS));
	
	// Compile shaders from files and create shader object
	HR(CompileShader(L"PointLightPixelShader.hlsl", "main", "ps_5_0", &m_pPS_Buffer));
	HR(m_pD3D11Device->CreatePixelShader(m_pPS_Buffer->GetBufferPointer(), m_pPS_Buffer->GetBufferSize(), NULL, &m_pPS));

	// Set the vertex and the pixel shader
	m_pD3D11DeviceContext->VSSetShader(m_pVS, 0, 0);
	m_pD3D11DeviceContext->PSSetShader(m_pPS, 0, 0);

	// ---------------------------------------------------------------------------
	// Cubes

	// Create the vertex buffer
	Vertex v[] =
	{
		// Front Face
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f),
		Vertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f),
		Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f),

		// Back Face
		Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f),
		Vertex(1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f),
		Vertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f),

		// Top Face
		Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, -1.0f),
		Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f),

		// Bottom Face
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f),
		Vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f),
		Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f),

		// Left Face
		Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 1.0f),
		Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 1.0f, -1.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f),

		// Right Face
		Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f),
		Vertex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f),
		Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f),
	};

	// Create the index buffer
	DWORD indices[] = {
		// Front Face
		0, 1, 2,
		0, 2, 3,

		// Back Face
		4, 5, 6,
		4, 6, 7,

		// Top Face
		8, 9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};

	// ---------------------------------------------------------------------------

	// Vertex buffer description
	D3D11_BUFFER_DESC vertexBufferDescription;
	ZeroMemory(&vertexBufferDescription, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	vertexBufferDescription.ByteWidth		= sizeof(Vertex) * 24;
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

	// ---------------------------------------------------------------------------
	// Ground

	// Create the vertex buffer
	Vertex groundVertices[] =
	{
		// Bottom Face
		Vertex(-1.0f, -1.0f, -1.0f, 100.0f, 100.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f, -1.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(-1.0f, -1.0f, 1.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f),
	};

	// Create the index buffer
	DWORD groundIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	// Vertex buffer description
	ZeroMemory(&vertexBufferDescription, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDescription.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescription.CPUAccessFlags = 0;
	vertexBufferDescription.MiscFlags = 0;

	// Index buffer description
	ZeroMemory(&indexBufferDescription, sizeof(D3D11_BUFFER_DESC));
	indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDescription.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescription.CPUAccessFlags = 0;
	indexBufferDescription.MiscFlags = 0;

	// Set vertex buffer data
	ZeroMemory(&vertexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexBufferData.pSysMem = groundVertices;

	// Set index buffer data
	ZeroMemory(&indexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexBufferData.pSysMem = groundIndices;

	// Create vertex buffer
	m_pD3D11Device->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &m_pGroundVertexBuffer);

	// Create index buffer
	m_pD3D11Device->CreateBuffer(&indexBufferDescription, &indexBufferData, &m_pGroundIndexBuffer);

	// ---------------------------------------------------------------------------

	// Set the vertex buffer
	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uiStride, &m_uiOffset);

	// Set the index buffer
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	m_pD3D11Device->CreateInputLayout(layout, numElements, m_pVS_Buffer->GetBufferPointer(), m_pVS_Buffer->GetBufferSize(), &m_pVertexLayout);

	// Set the input layout
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout);

	// Set primitive topology
	m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ---------------------------------------------------------------------------
	// Create constant buffer per object - send WVP matrix

	D3D11_BUFFER_DESC constBufferDescription;
	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage			= D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth		= sizeof(m_cbPerObjectStruct);
	constBufferDescription.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags	= 0;
	constBufferDescription.MiscFlags		= 0;

	HR(m_pD3D11Device->CreateBuffer(&constBufferDescription, NULL, &m_cbPerObjectBuffer));

	// ---------------------------------------------------------------------------
	// Create constant buffer per frame - send directional light data

	ZeroMemory(&constBufferDescription, sizeof(D3D11_BUFFER_DESC));

	constBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	constBufferDescription.ByteWidth = sizeof(m_cbPerFrameStruct);
	constBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.CPUAccessFlags = 0;
	constBufferDescription.MiscFlags = 0;

	HR(m_pD3D11Device->CreateBuffer(&constBufferDescription, NULL, &m_cbPerFrameBuffer));

	// ---------------------------------------------------------------------------

	// Create the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX	= 0;
	viewport.TopLeftY	= 0;
	viewport.Width		= (FLOAT)m_iClientWidth;
	viewport.Height		= (FLOAT)m_iClientHeight;
	viewport.MinDepth	= 0.0f;
	viewport.MaxDepth	= 1.0f;

	// Set the viewport
	m_pD3D11DeviceContext->RSSetViewports(1, &viewport);

	// ---------------------------------------------------------------------------

	// Load textures from file
	HR(CreateWICTextureFromFile(m_pD3D11Device,
		m_pD3D11DeviceContext,
		L"braynzar.jpg",
		nullptr, 
		&m_pCubeTexture));

	HR(CreateWICTextureFromFile(m_pD3D11Device,
		m_pD3D11DeviceContext,
		L"grass.jpg",
		nullptr,
		&m_pGroundTexture));

	// Fill in the sampler description
	D3D11_SAMPLER_DESC samplerDescription; // Describes how the shader will render the texture
	ZeroMemory(&samplerDescription, sizeof(D3D11_SAMPLER_DESC));
	samplerDescription.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescription.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.ComparisonFunc	= D3D11_COMPARISON_NEVER;
	samplerDescription.MinLOD			= 0;
	samplerDescription.MaxLOD			= D3D11_FLOAT32_MAX;

	// Create the sampler
	HR(m_pD3D11Device->CreateSamplerState(&samplerDescription, &m_pCubeTexSameplerState));

	// ---------------------------------------------------------------------------

	// Set up lighting
	// Directional light
	m_Light.Direction	= Vector3(0.0f, 1.0f, 0.0f);
	m_Light.Ambient		= Vector3(0.2f, 0.2f, 0.2f);
	m_Light.Diffuse		= Vector3(1.0f, 1.0f, 1.0f);

	// Point light
	m_Light.Position	= Vector3(0.0f, 0.0f, 0.0f);
	m_Light.Range		= 100.0f;
	m_Light.Attenuation = Vector3(0.0f, 0.2f, 0.0f);
	m_Light.Ambient		= Vector3(0.3f, 0.3f, 0.3f);
	m_Light.Diffuse		= Vector3(1.0f, 1.0f, 1.0f);

	return true;
}

void D3DApp::UpdateScene(double dt)
{
	// ---------------------------------------------------------------------------
	// Call base update

	BaseD3D::UpdateScene(dt);

	// ---------------------------------------------------------------------------

	// Rotate the cubes
	m_fRotation += (float)(1.0f * dt);
	if (m_fRotation > DirectX::XM_2PI)
	{
		m_fRotation = 0.0f;
	}

	// Reset cube1 world
	m_mCube1World = Matrix();
	// Define cube1 world matrix
	m_mRotation		= Matrix::CreateRotationY(m_fRotation);
	m_mTranslation	= Matrix::CreateTranslation(0.0f, 0.0f, 4.0f);
	m_mCube1World = m_mTranslation * m_mRotation;

	// Reset cube2 world
	m_mCube2World = Matrix();
	// Define cube2 world matrix
	m_mRotation		= Matrix::CreateRotationY(-m_fRotation);
	m_mCube2World	= m_mRotation;

	// Reset groundWorld matrix
	m_mGroundWorld = Matrix();
	// Define ground world matrix
	Matrix groundScale = Matrix::CreateScale(500.0f, 1.0f, 500.0f);
	Matrix groundTranslation = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
	m_mGroundWorld = groundScale * groundTranslation;

	// Update the position of the point light to the position of the cube1
	Vector4 pointLightPosition = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	pointLightPosition = Vector4::Transform(pointLightPosition, m_mCube1World);
	m_Light.Position.x = pointLightPosition.x;
	m_Light.Position.y = pointLightPosition.y;
	m_Light.Position.z = pointLightPosition.z;
}

void D3DApp::DrawScene(int iFPS, double dFrameTime)
{
	// ---------------------------------------------------------------------------
	// Set the clear color
	float bgColor[4];
	bgColor[0] = red;
	bgColor[1] = green;
	bgColor[2] = blue;
	bgColor[3] = 1.0f;

	Draw(m_pCamera->View(), m_pCamera->PerpectiveProjection(), bgColor);

	// ---------------------------------------------------------------------------
	// Call base update

	BaseD3D::DrawScene(iFPS, dFrameTime);

	// ---------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------

void D3DApp::Draw(const Matrix& view, const Matrix& projection, float color[4])
{
	// Set render target
	m_pD3D11DeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// ---------------------------------------------------------------------------

	// Clear the render target view
	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	// Clear the depth/stencil view
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, // Value to set in the depth buffer
		0); // Value to set the stencil buffer

	// ---------------------------------------------------------------------------

	// Draw 3D scene

	// Set state
	m_pD3D11DeviceContext->VSSetShader(m_pVS, 0, 0);
	m_pD3D11DeviceContext->PSSetShader(m_pPS, 0, 0);
	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uiStride, &m_uiOffset);
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout);
	m_pD3D11DeviceContext->OMSetDepthStencilState(NULL, 0);

	// ---------------------------------------------------------------------------
	// Set the light

	// Update the structure which is being sent to GPU
	m_cbPerFrameStruct.light = m_Light;
	// Update res in the GPU buffer
	m_pD3D11DeviceContext->UpdateSubresource(m_cbPerFrameBuffer, 0, NULL, &m_cbPerFrameStruct, 0, 0);
	// Sent the buffer to the GPU
	m_pD3D11DeviceContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);

	// ---------------------------------------------------------------------------

	RenderModels(view * projection);

	// ---------------------------------------------------------------------------
}

void D3DApp::RenderModels(const Matrix& viewProjection)
{
	// Cube 1

	// Set the world/view/projection matrix
	m_mWVP = m_mCube1World * viewProjection;
	m_mWVP.Transpose(m_cbPerObjectStruct.WVP);
	m_mCube1World.Transpose(m_cbPerObjectStruct.World);
	m_pD3D11DeviceContext->UpdateSubresource(m_cbPerObjectBuffer, 0, NULL, &m_cbPerObjectStruct, 0, 0);
	m_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_cbPerObjectBuffer);
	// Set the render state
	m_pD3D11DeviceContext->RSSetState(NULL);
	// Set the texture data
	m_pD3D11DeviceContext->PSSetShaderResources(0, 1, &m_pCubeTexture);
	m_pD3D11DeviceContext->PSSetSamplers(0, 1, &m_pCubeTexSameplerState);
	// Render the cube1
	m_pD3D11DeviceContext->DrawIndexed(36, 0, 0);
	// ---------------------------------------------------------------------------

	// Cube 2

	// Set the world/view/projection matrix
	m_mWVP = m_mCube2World * viewProjection;
	m_mWVP.Transpose(m_cbPerObjectStruct.WVP);
	m_mCube2World.Transpose(m_cbPerObjectStruct.World);
	m_pD3D11DeviceContext->UpdateSubresource(m_cbPerObjectBuffer, 0, NULL, &m_cbPerObjectStruct, 0, 0);
	m_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_cbPerObjectBuffer);
	// Set the render state
	m_pD3D11DeviceContext->RSSetState(NULL);
	// Set the texture data
	m_pD3D11DeviceContext->PSSetShaderResources(0, 1, &m_pCubeTexture);
	m_pD3D11DeviceContext->PSSetSamplers(0, 1, &m_pCubeTexSameplerState);
	// Render the cube2
	m_pD3D11DeviceContext->DrawIndexed(36, 0, 0);

	// ---------------------------------------------------------------------------

	// Ground

	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pGroundVertexBuffer, &m_uiStride, &m_uiOffset);
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the world/view/projection matrix
	m_mWVP = m_mGroundWorld * viewProjection;
	m_mWVP.Transpose(m_cbPerObjectStruct.WVP);
	m_mGroundWorld.Transpose(m_cbPerObjectStruct.World);
	m_pD3D11DeviceContext->UpdateSubresource(m_cbPerObjectBuffer, 0, NULL, &m_cbPerObjectStruct, 0, 0);
	m_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_cbPerObjectBuffer);
	// Set the render state
	m_pD3D11DeviceContext->RSSetState(m_pCCWCullMode);
	//m_pD3D11DeviceContext->RSSetState(NULL);
	// Set the texture data
	m_pD3D11DeviceContext->PSSetShaderResources(0, 1, &m_pGroundTexture);
	m_pD3D11DeviceContext->PSSetSamplers(0, 1, &m_pCubeTexSameplerState);
	// Render the ground
	m_pD3D11DeviceContext->DrawIndexed(6, 0, 0);

	// ---------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------