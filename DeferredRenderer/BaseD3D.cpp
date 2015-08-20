#include "BaseD3D.h"

#include "TextRenderer.h"

// ----------------------------------------------------------------------------

BaseD3D::BaseD3D()
{
}

// ----------------------------------------------------------------------------

BaseD3D::~BaseD3D()
{
}

// ----------------------------------------------------------------------------

bool BaseD3D::InitializeDirect3D11(HWND hwnd, HINSTANCE hInstance, unsigned int width, unsigned int height, bool fullScreen)
{
	// ---------------------------------------------------------------------------

	// Initialize client width and height
	m_iClientHeight = height;
	m_iClientWidth = width;
	m_HWND = hwnd;

	// ---------------------------------------------------------------------------

	// Create the device object and the device context
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;

	HR(D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&m_pD3D11Device,
		&featureLevel,
		&m_pD3D11DeviceContext));

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D 11 not supported", L"Error", MB_OK);
		return false;
	}

	// ---------------------------------------------------------------------------
	// Create the swap chain

	// Buffer description structure
	DXGI_MODE_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(DXGI_MODE_DESC));

	bufferDescription.Width = width;
	bufferDescription.Height = height;
	bufferDescription.RefreshRate.Numerator = 60;
	bufferDescription.RefreshRate.Denominator = 1;
	bufferDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDescription.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Swap chain description
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDescription;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = !fullScreen;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Check for multisampling support
	m_iMSAACount = 4;
	HR(m_pD3D11Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_iMSAACount, &m_iMSAAQuality));
	assert(m_iMSAAQuality > 0);
	if (m_bEnable4xMsaa)
	{
		swapChainDesc.SampleDesc.Count = m_iMSAACount;
		swapChainDesc.SampleDesc.Quality = m_iMSAAQuality - 1;
	}

	// ---------------------------------------------------------------------------
	// Create the swap chain

	IDXGIDevice* dxgiDevice = 0;
	HR(m_pD3D11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));
	// Get the IDXGIFactory interface.
	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	// Create the swap chain.
	HR(dxgiFactory->CreateSwapChain(m_pD3D11Device, &swapChainDesc, &m_pSwapChain));

	// Release our acquired COM interfaces (because we are done with them).
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// ---------------------------------------------------------------------------

	TextRenderer::GetInstance().Initialize(m_pD3D11DeviceContext, m_pD3D11Device);

	// ---------------------------------------------------------------------------

	return true;
}

// ----------------------------------------------------------------------------

void BaseD3D::Shutdown()
{
	m_pSwapChain->SetFullscreenState(false, NULL);
	PostMessage(m_HWND, WM_DESTROY, 0, 0);

	// D3D11
	m_pSwapChain->Release();
	m_pD3D11Device->Release();
	m_pD3D11DeviceContext->Release();
}

void BaseD3D::UpdateScene(double dt)
{

}

// ----------------------------------------------------------------------------

void BaseD3D::DrawScene(int iFPS, double dFrameTime)
{
	// ---------------------------------------------------------------------------

	// Render Gfx stats
	TextRenderer::GetInstance().RenderText(iFPS, dFrameTime);

	// ---------------------------------------------------------------------------

	// Present the back buffer to the screen
	m_pSwapChain->Present(0, 0);

	// ---------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------