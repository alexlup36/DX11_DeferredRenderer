#include <windows.h>

// ----------------------------------------------------------------------------
// Global variables -----------------------------------------------------------
// ----------------------------------------------------------------------------

LPCTSTR WndClassName = "Win32Window";
HWND hwnd = NULL;

const int Width = 800;
const int Height = 600;

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
	// If failed to initialize the window
	if (InitializeWindow(hInstance, nShowCmd, Width, Height, true) == false)
	{
		MessageBox(0, "Window initialization failed", "Error", MB_OK);
		return 0;
	}

	messageloop();

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