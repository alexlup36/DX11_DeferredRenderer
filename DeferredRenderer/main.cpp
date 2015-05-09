#include <memory>

// ----------------------------------------------------------------------------
// Includes -------------------------------------------------------------------
// ----------------------------------------------------------------------------

#include "D3DApp.h"

//#include "CommonStates.h"
//#include "DDSTextureLoader.h"
//#include "DirectXHelpers.h"
//#include "Effects.h"
//#include "GamePad.h"
//#include "GeometricPrimitive.h"
//#include "Model.h"
//#include "PrimitiveBatch.h"
//#include "ScreenGrab.h"
//#include "SpriteBatch.h"
//#include "SpriteFont.h"
//#include "VertexTypes.h"
//#include "WICTextureLoader.h"


// ----------------------------------------------------------------------------
// Global variables -----------------------------------------------------------
// ----------------------------------------------------------------------------

LPCTSTR WndClassName = "Win32Window";
HWND hwnd = NULL;

const int Width = 1366;
const int Height = 768;

const int FullScreenWidth = 1920;
const int FullScreenHeight = 1080;

const bool bFullScreen = true;

std::shared_ptr<D3DApp> d3dapp = std::make_shared<D3DApp>();

// ----------------------------------------------------------------------------
// Function prototypes --------------------------------------------------------
// ----------------------------------------------------------------------------

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed);

// ----------------------------------------------------------------------------

int messageloop();

// ----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

// ----------------------------------------------------------------------------
// Main -----------------------------------------------------------------------
// ----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	if (bFullScreen)
	{
		// If failed to initialize the window
		if (InitializeWindow(hInstance, nShowCmd, FullScreenWidth, FullScreenHeight, true) == false)
		{
			MessageBox(0, "Window initialization failed", "Error", MB_OK);
			return 0;
		}

		// Initialize Direct3D
		if (d3dapp->InitializeDirect3D11(hwnd, hInstance, FullScreenWidth, FullScreenHeight, bFullScreen) == false)
		{
			MessageBox(0, "Failed to initialize Direct3D", "Error", MB_OK);
			return 0;
		}
	}
	else
	{
		// If failed to initialize the window
		if (InitializeWindow(hInstance, nShowCmd, Width, Height, true) == false)
		{
			MessageBox(0, "Window initialization failed", "Error", MB_OK);
			return 0;
		}

		// Initialize Direct3D
		if (d3dapp->InitializeDirect3D11(hwnd, hInstance, Width, Height, bFullScreen) == false)
		{
			MessageBox(0, "Failed to initialize Direct3D", "Error", MB_OK);
			return 0;
		}
	}
	

	// Initialize scene
	if (d3dapp->InitScene() == false)
	{
		MessageBox(0, "Failed to initialize the scene", "Error", MB_OK);
		return 0;
	}

	messageloop();

	// Cleanup
	d3dapp->ReleaseObjects();

	return 0;
}

// ----------------------------------------------------------------------------
// Function implementation ----------------------------------------------------
// ----------------------------------------------------------------------------

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2); // Black
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// Register class
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Error registering class", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	DWORD error = GetLastError();

	// Create window
	hwnd = CreateWindowEx(NULL,
		WndClassName, /* use the previously registered class name */
		"DirectX11",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, /* start x start y*/
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL);

	error = GetLastError();

	// Failed to create window?
	if (hwnd == false)
	{
		MessageBox(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Show window
	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

// ----------------------------------------------------------------------------

int messageloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Run application
			d3dapp->UpdateScene();
			d3dapp->DrawScene();
		}
	}

	return msg.wParam;
}

// ----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				if (MessageBox(0, "Are you sure you want to quit?",
					"Really?",
					MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					DestroyWindow(hwnd);
				}
			}

			break;
		}
		
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}	
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------