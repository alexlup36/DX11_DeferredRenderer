#ifndef BASED3D_H
#define BASED3D_H

#include <assert.h>
#include <memory>
#include <windows.h>
#include <d3d11.h>

#include "DXUtil.h"

class BaseD3D
{
public:
	BaseD3D();
	virtual ~BaseD3D();

	bool InitializeDirect3D11(HWND hwnd,
		HINSTANCE hInstance,
		unsigned int width,
		unsigned int height,
		bool fullScreen);
	void Shutdown();

	void UpdateScene(double dt);
	void DrawScene(int iFPS, double dFrameTime);

protected:

	IDXGISwapChain*			m_pSwapChain;
	ID3D11Device*			m_pD3D11Device;
	ID3D11DeviceContext*	m_pD3D11DeviceContext;
	ID3D11RenderTargetView* m_pRenderTargetView;
	HWND					m_HWND;
	bool					m_bEnable4xMsaa = true;
	// Screen depth
	float m_fScreenDepth;
	float m_fScreenNear;

	// Near plane width
	float m_fNearPlaneWidth = 0.4f;

	// Screen size
	int m_iClientWidth;
	int m_iClientHeight;

	// MSAA
	UINT m_iMSAACount;
	UINT m_iMSAAQuality;

	// Clear color
	float red		= 0.0f;
	float green		= 0.0f;
	float blue		= 0.2f;
};

#endif // BASED3D_H