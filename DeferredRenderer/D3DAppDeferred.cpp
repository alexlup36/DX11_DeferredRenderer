#include "D3DAppDeferred.h"

#include <d3dcompiler.h>
#include <string>
#include "WICTextureLoader.h"
#include "Mesh.h"

D3DAppDeferred::D3DAppDeferred()
{
	m_pDeferredBuffers			= nullptr;
	m_pDeferredRenderToTexture	= nullptr;
	m_pDeferredRenderer			= nullptr;

	m_pDepthStencilStateEnable	= nullptr;
	m_pDepthStencilStateDisable = nullptr;
}


D3DAppDeferred::~D3DAppDeferred()
{
}

bool D3DAppDeferred::InitializeDirect3D11(HWND hwnd,
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

	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
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

void D3DAppDeferred::Shutdown()
{
	// ---------------------------------------------------------------------------

	BaseD3D::Shutdown();

	// ---------------------------------------------------------------------------

	// Rendering
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();

	// Release depth/stencil buffer/view
	m_pDepthStencilView->Release();
	m_pDepthStencilBuffer->Release();

	// Render state
	m_pWireFrameRenderState->Release();
	m_pCCWCullMode->Release();
	m_pCWCullMode->Release();

	// Deferred rendering shutdown
	if (m_pDeferredBuffers != nullptr)
	{
		m_pDeferredBuffers->Shutdown();
		delete m_pDeferredBuffers;
		m_pDeferredBuffers = nullptr;
	}

	if (m_pDeferredRenderer != nullptr)
	{
		m_pDeferredRenderer->Shutdown();
		delete m_pDeferredRenderer;
		m_pDeferredRenderer = nullptr;
	}

	if (m_pDeferredRenderToTexture != nullptr)
	{
		m_pDeferredRenderToTexture->Shutdown();
		delete m_pDeferredRenderToTexture;
		m_pDeferredRenderToTexture = nullptr;
	}

	// Depth stencil states
	m_pDepthStencilStateEnable->Release();
	m_pDepthStencilStateDisable->Release();
}

bool D3DAppDeferred::InitScene()
{
	// ---------------------------------------------------------------------------

	m_fScreenDepth = 5000.0f;
	m_fScreenNear = 0.001f;

	// ---------------------------------------------------------------------------

	// Create camera instance
	m_pCamera = std::make_unique<Camera>(m_fNearPlaneWidth,
		m_fScreenNear,
		m_fScreenDepth,
		m_iClientWidth,
		m_iClientHeight);
	m_pCamera->SetPosition(Vector4(0.0f, 50.0f, 0.0f, 1.0f));

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

	// Create the viewport
	ZeroMemory(&m_Viewport, sizeof(D3D11_VIEWPORT));
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (FLOAT)m_iClientWidth;
	m_Viewport.Height = (FLOAT)m_iClientHeight;
	// Om stage needs to convert the depth values between 0 - 1
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	// Set the viewport
	m_pD3D11DeviceContext->RSSetViewports(1, &m_Viewport);

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

	// ---------------------------------------------------------------------------

	// Set up lighting

	for (int iPointLightIndex = 0; iPointLightIndex < MAXPOINTLIGHTS; iPointLightIndex++)
	{
		// Directional light
		//m_Light[iPointLightIndex].Direction = Vector3(0.0f, 1.0f, 0.0f);
		//m_Light[iPointLightIndex].Ambient = Vector3(0.0f, 0.0f, 0.0f);
		//m_Light[iPointLightIndex].Diffuse = Vector3(1.0f, 1.0f, 1.0f);

		// Point light
		m_Light[iPointLightIndex].Position		= Vector3(0.0f, 0.0f, 0.0f);
		m_Light[iPointLightIndex].Range = Random(3.0f, 5.0f);
		m_Light[iPointLightIndex].Attenuation = Vector3(Random(1.0f), Random(1.0f), Random(1.0f));

		m_Light[iPointLightIndex].Ambient = Vector3(Random(1.0f), Random(1.0f), Random(1.0f));
		m_Light[iPointLightIndex].Diffuse = Vector3(Random(1.0f), Random(1.0f), Random(1.0f));
		m_Light[iPointLightIndex].Specular = Vector3(Random(1.0f), Random(1.0f), Random(1.0f));
	}

	// ---------------------------------------------------------------------------

	// Setup render to texture
	RenderToTextureSetup();

	// Initialize deferred rendering

	// Create the deferred buffers class
	m_pDeferredBuffers = new DeferredBuffers();
	m_pDeferredBuffers->Initialize(m_pD3D11Device, 
		m_iClientWidth, 
		m_iClientHeight, 
		m_fScreenDepth, 
		m_fScreenNear,
		m_iMSAACount,
		m_iMSAAQuality);

	// Create the class responsible for rendering to the GBUffer
	m_pDeferredRenderToTexture = new DeferredRenderToTexture();
	m_pDeferredRenderToTexture->Initialize(m_pD3D11Device);

	// Create the deferred rendering class
	m_pDeferredRenderer = new DeferredRenderer();
	m_pDeferredRenderer->Initialize(m_pD3D11Device);

	// ---------------------------------------------------------------------------

	// Initialize the depth stencil states

	D3D11_DEPTH_STENCIL_DESC enableDepthStencilDescription;
	ZeroMemory(&enableDepthStencilDescription, sizeof(enableDepthStencilDescription));

	enableDepthStencilDescription.DepthEnable					= true;
	enableDepthStencilDescription.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;
	enableDepthStencilDescription.DepthFunc						= D3D11_COMPARISON_LESS;
	enableDepthStencilDescription.StencilEnable					= true;
	enableDepthStencilDescription.StencilReadMask				= 0xFF;
	enableDepthStencilDescription.StencilWriteMask				= 0xFF;
	// Stencil operations if pixel is front-facing.
	enableDepthStencilDescription.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	enableDepthStencilDescription.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
	enableDepthStencilDescription.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	enableDepthStencilDescription.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	enableDepthStencilDescription.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	enableDepthStencilDescription.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	enableDepthStencilDescription.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	enableDepthStencilDescription.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state for enabling Z buffering
	HR(m_pD3D11Device->CreateDepthStencilState(&enableDepthStencilDescription, &m_pDepthStencilStateEnable));

	D3D11_DEPTH_STENCIL_DESC disableDepthStencilDescription;
	ZeroMemory(&disableDepthStencilDescription, sizeof(disableDepthStencilDescription));

	disableDepthStencilDescription.DepthEnable						= false;
	disableDepthStencilDescription.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
	disableDepthStencilDescription.DepthFunc						= D3D11_COMPARISON_LESS;
	disableDepthStencilDescription.StencilEnable					= true;
	disableDepthStencilDescription.StencilReadMask					= 0xFF;
	disableDepthStencilDescription.StencilWriteMask					= 0xFF;
	// Stencil operations if pixel is front-facing.
	disableDepthStencilDescription.FrontFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	disableDepthStencilDescription.FrontFace.StencilDepthFailOp		= D3D11_STENCIL_OP_INCR;
	disableDepthStencilDescription.FrontFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	disableDepthStencilDescription.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	disableDepthStencilDescription.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	disableDepthStencilDescription.BackFace.StencilDepthFailOp		= D3D11_STENCIL_OP_DECR;
	disableDepthStencilDescription.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	disableDepthStencilDescription.BackFace.StencilFunc				= D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state for disabling Z buffering
	HR(m_pD3D11Device->CreateDepthStencilState(&disableDepthStencilDescription, &m_pDepthStencilStateDisable));

	// Set the depth stencil state
	EnableZBuffering();

	// ---------------------------------------------------------------------------
	// Load models
	//Mesh* testMesh = new Mesh();
	//testMesh->LoadOBJ("cube.obj", false);

	// ---------------------------------------------------------------------------

	return true;
}

void D3DAppDeferred::UpdateScene(double dt)
{
	// ---------------------------------------------------------------------------
	// Call base update

	BaseD3D::UpdateScene(dt);

	// ---------------------------------------------------------------------------

	// Rotate the cubes
	m_fRotation += (float)(2.0f * dt);
	if (m_fRotation > DirectX::XM_2PI)
	{
		m_fRotation = 0.0f;
	}

	// Reset groundWorld matrix
	m_mGroundWorld = Matrix();
	// Define ground world matrix
	Matrix groundScale = Matrix::CreateScale(500.0f, 1.0f, 500.0f);
	Matrix groundTranslation = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
	m_mGroundWorld = groundScale * groundTranslation;	
}

void D3DAppDeferred::DrawScene(int iFPS, double dFrameTime)
{
	// Clear the back buffer
	float bgColor[4];
	bgColor[0] = red;
	bgColor[1] = green;
	bgColor[2] = blue;
	bgColor[3] = 1.0f;

	// ---------------------------------------------------------------------------
	// Enable z buffering
	EnableZBuffering();

	RenderToTexture(bgColor);

	// Render to back buffer
	RenderToBackBuffer(bgColor);

	// ---------------------------------------------------------------------------
	// Call base update

	BaseD3D::DrawScene(iFPS, dFrameTime);

	// ---------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------
// Render to texture setup

void D3DAppDeferred::RenderToTextureSetup()
{
	// ---------------------------------------------------------------------------

	// Setup the vertex buffer
	VertexDeferred v[] =
	{
		// Front Face
		/*Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f),
		Vertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f),
		Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f),*/

		VertexDeferred(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		VertexDeferred(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f),
		VertexDeferred(1.0f, 1.0f, -1.0f, 1.0f, 0.0f),
		VertexDeferred(1.0f, -1.0f, -1.0f, 1.0f, 1.0f),
	};

	DWORD indices[] = 
	{
		// Front Face
		0, 1, 2,
		0, 2, 3,
	};

	// ---------------------------------------------------------------------------
	// Create index buffer

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	HR(m_pD3D11Device->CreateBuffer(&indexBufferDesc, &iinitData, &m_pQuadIndexBuffer));

	// ---------------------------------------------------------------------------
	// Create vertex buffer

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexDeferred) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	HR(m_pD3D11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_pQuadVertexBuffer));
}

// ----------------------------------------------------------------------------

void D3DAppDeferred::RenderToBackBuffer(float color[4])
{
	// ---------------------------------------------------------------------------
	// Render to back buffer

	// Clear the back buffer.
	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// Clear the depth buffer.
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set the render state
	m_pD3D11DeviceContext->RSSetState(m_pWireFrameRenderState);

	// Turn off z buffering
	DisableZBuffering();

	// Render the textured quad on the screen
	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &m_uiStrideDeferredVertex, &m_uiOffset);
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render to quad using the g buffers

	// Cube 1
	Matrix world = Matrix::Identity;
	Matrix view = Matrix::Identity;
	Matrix wvp = /*m_pCamera->PerpectiveProjection() * */view * world;

	m_pDeferredRenderer->UpdateScene(wvp,
		m_pDeferredBuffers->GetShaderResourceView(BUFFERTYPE::keNormal),
		m_pDeferredBuffers->GetShaderResourceView(BUFFERTYPE::keDiffuseAlbedo),
		m_pDeferredBuffers->GetShaderResourceView(BUFFERTYPE::keSpecularAlbedo),
		m_pDeferredBuffers->GetShaderResourceView(BUFFERTYPE::kePosition),
		Vector3(m_pCamera->Position()));

	m_pDeferredRenderer->DrawScene(m_pD3D11DeviceContext, 6, m_Light);
}

// ----------------------------------------------------------------------------

void D3DAppDeferred::RenderToTexture(float color[4])
{
	// Draw the scene to the render texture from the orthographic camera perspective

	// RENDER TARGETS ------------------------------------------------------------
	// Set the render target
	m_pDeferredBuffers->SetRenderTargets(m_pD3D11DeviceContext);

	// Clear the render target
	m_pDeferredBuffers->ClearRenderTargets(m_pD3D11DeviceContext, color);

	// Set primitive topology
	m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// CUBE ----------------------------------------------------------------------
	// Set the vertex and index buffers for the cube model
	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uiStride, &m_uiOffset);
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pD3D11DeviceContext->RSSetState(NULL);

	for (int iPointLightHeight = 0; iPointLightHeight < MAXPOINTLIGHTS_HEIGHT; iPointLightHeight++)
	{
		for (int iPointLightWidth = 0; iPointLightWidth < MAXPOINTLIGHTS_WIDTH; iPointLightWidth++)
		{
			Matrix cubeWorld = Matrix();
			Matrix pointLightWorld = Matrix();

			m_mRotation = Matrix::CreateRotationY(m_fRotation);
			m_mTranslation = Matrix::CreateTranslation(iPointLightWidth * 10.0f, 0.0f, iPointLightHeight * 10.0f);

			cubeWorld = m_mRotation * m_mTranslation;
			pointLightWorld = Matrix::CreateTranslation(5.0f, 0.0f, 0.0f) * m_mRotation * m_mTranslation;
			m_mWVP = cubeWorld * m_pCamera->View() * m_pCamera->PerpectiveProjection();

			m_pDeferredRenderToTexture->UpdateScene(m_mWVP, cubeWorld, m_pCubeTexture, m_vSpecularAlbedo, m_fSpecularPower);

			// Update the position of the point light to the position of the cube
			Vector4 pointLightPosition = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			pointLightPosition = Vector4::Transform(pointLightPosition, pointLightWorld);

			int iLightIndex = iPointLightHeight * MAXPOINTLIGHTS_WIDTH + iPointLightWidth;
			m_Light[iLightIndex].Position.x = pointLightPosition.x;
			m_Light[iLightIndex].Position.y = pointLightPosition.y;
			m_Light[iLightIndex].Position.z = pointLightPosition.z;

			m_pDeferredRenderToTexture->Draw(m_pD3D11DeviceContext, 36);
		}
	}

	// GROUND --------------------------------------------------------------------
	// Set the vertex and index buffers for the ground
	m_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_pGroundVertexBuffer, &m_uiStride, &m_uiOffset);
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pD3D11DeviceContext->RSSetState(m_pCCWCullMode);
	
	// Set the world.view.projection matrix
	m_mWVP = m_mGroundWorld * m_pCamera->View() * m_pCamera->PerpectiveProjection();
	m_pDeferredRenderToTexture->UpdateScene(m_mWVP, m_mGroundWorld, m_pGroundTexture, m_vSpecularAlbedo, m_fSpecularPower);
	m_pDeferredRenderToTexture->Draw(m_pD3D11DeviceContext, 6);

	// Set the render target and the depth stencil buffer
	m_pD3D11DeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	// Reset the viewport
	m_pD3D11DeviceContext->RSSetViewports(1, &m_Viewport);
}

// ----------------------------------------------------------------------------

void D3DAppDeferred::EnableZBuffering()
{
	m_pD3D11DeviceContext->OMSetDepthStencilState(m_pDepthStencilStateEnable, 1);
}

// ----------------------------------------------------------------------------

void D3DAppDeferred::DisableZBuffering()
{
	m_pD3D11DeviceContext->OMSetDepthStencilState(m_pDepthStencilStateDisable, 1);
}

// ----------------------------------------------------------------------------