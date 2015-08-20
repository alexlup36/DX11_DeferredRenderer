#include <memory>
#include <iostream>

#ifdef _DEBUG
	// Add IO console
	#include <stdio.h>
	#include <io.h>
	#include <fcntl.h>
#endif // _DEBUG

// ----------------------------------------------------------------------------
// Includes -------------------------------------------------------------------
// ----------------------------------------------------------------------------

#include "D3DApp.h"
#include "D3DAppDeferred.h"
#include "Timer.h"
#include "Input.h"

// ----------------------------------------------------------------------------
// Global variables -----------------------------------------------------------
// ----------------------------------------------------------------------------

LPCTSTR WndClassName = L"Win32Window";
HWND hwnd = NULL;

const int Width = 1280;
const int Height = 720;

const int FullScreenWidth = 1920;
const int FullScreenHeight = 1080;

const bool bFullScreen = false;
const bool bDeferredRendering = true;

int iFPS = 0;

std::shared_ptr<D3DApp> d3dapp = std::make_shared<D3DApp>();
std::shared_ptr<D3DAppDeferred> d3dappDeferred = std::make_shared<D3DAppDeferred>();

std::unique_ptr<Timer> pTimer = std::make_unique<Timer>();

// ----------------------------------------------------------------------------
// Function prototypes --------------------------------------------------------
// ----------------------------------------------------------------------------

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed);

#ifdef _DEBUG
void AddConsole();
#endif

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
#ifdef _DEBUG
	AddConsole();
#endif

	if (bFullScreen)
	{
		// If failed to initialize the window
		if (InitializeWindow(hInstance, nShowCmd, FullScreenWidth, FullScreenHeight, true) == false)
		{
			MessageBox(0, L"Window initialization failed", L"Error", MB_OK);
			return 0;
		}

		if (bDeferredRendering == true)
		{
			// Initialize Direct3D deferred version
			if (d3dappDeferred->InitializeDirect3D11(hwnd, hInstance, FullScreenWidth, FullScreenHeight, bFullScreen) == false)
			{
				MessageBox(0, L"Failed to initialize Direct3D", L"Error", MB_OK);
				return 0;
			}
		}
		else
		{
			// Initialize Direct3D
			if (d3dapp->InitializeDirect3D11(hwnd, hInstance, FullScreenWidth, FullScreenHeight, bFullScreen) == false)
			{
				MessageBox(0, L"Failed to initialize Direct3D", L"Error", MB_OK);
				return 0;
			}
		}
	}
	else
	{
		// If failed to initialize the window
		if (InitializeWindow(hInstance, nShowCmd, Width, Height, true) == false)
		{
			MessageBox(0, L"Window initialization failed", L"Error", MB_OK);
			return 0;
		}

		if (bDeferredRendering == true)
		{
			// Initialize Direct3D deferred version
			if (d3dappDeferred->InitializeDirect3D11(hwnd, hInstance, Width, Height, bFullScreen) == false)
			{
				MessageBox(0, L"Failed to initialize Direct3D", L"Error", MB_OK);
				return 0;
			}
		}
		else
		{
			// Initialize Direct3D
			if (d3dapp->InitializeDirect3D11(hwnd, hInstance, Width, Height, bFullScreen) == false)
			{
				MessageBox(0, L"Failed to initialize Direct3D", L"Error", MB_OK);
				return 0;
			}
		}
	}
	

	// Initialize scene
	if (bDeferredRendering == true)
	{
		// Init scene for deferred rendering
		if (d3dappDeferred->InitScene() == false)
		{
			MessageBox(0, L"Failed to initialize the scene", L"Error", MB_OK);
			return 0;
		}

		// Initialize DirectInput
		if (Input::GetInstance().InitializeDirectInput(hwnd, hInstance, d3dappDeferred->GetActiveCamera()) == false)
		{
			MessageBox(0, L"Failed to initialize the DirectInput", L"Error", MB_OK);
			return 0;
		}
	}
	else
	{
		// Init scene for forward rendering
		if (d3dapp->InitScene() == false)
		{
			MessageBox(0, L"Failed to initialize the scene", L"Error", MB_OK);
			return 0;
		}

		// Initialize DirectInput
		if (Input::GetInstance().InitializeDirectInput(hwnd, hInstance, d3dapp->GetActiveCamera()) == false)
		{
			MessageBox(0, L"Failed to initialize the DirectInput", L"Error", MB_OK);
			return 0;
		}
	}

	messageloop();

	// Cleanup
	if (bDeferredRendering == true)
	{
		d3dappDeferred->Shutdown();
	}
	else
	{
		d3dapp->Shutdown();
	}
	
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
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	DWORD error = GetLastError();

	// Create window
	hwnd = CreateWindowEx(NULL,
		WndClassName, /* use the previously registered class name */
		L"DirectX11",
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
		MessageBox(NULL, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
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
			pTimer->IncrementFrameCount();
			if (pTimer->GetTime() > 1.0f)
			{
				iFPS = pTimer->GetFrameCount();

				// Update timer
				pTimer->ResetFrameCount();
				pTimer->StartTimer();
			}

			double dFrameTime = pTimer->GetFrameTime();

			// Check for input
			Input::GetInstance().DetectInput(dFrameTime);

			// Run application
			if (bDeferredRendering)
			{
				d3dappDeferred->UpdateScene(dFrameTime);
				d3dappDeferred->DrawScene(iFPS, dFrameTime);
			}
			else
			{
				d3dapp->UpdateScene(dFrameTime);
				d3dapp->DrawScene(iFPS, dFrameTime);
			}
			
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
				if (bFullScreen == false)
				{
					if (MessageBox(0, L"Are you sure you want to quit?",
						L"Really?",
						MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						DestroyWindow(hwnd);
					}
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

#ifdef _DEBUG
void AddConsole()
{
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;
}
#endif

// ----------------------------------------------------------------------------