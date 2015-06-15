#include "Input.h"

#include <iostream>

// ---------------------------------------------------------------------------

Input::~Input()
{
	m_pDIKeyboard->Unacquire();
	m_pDIMouse->Unacquire();
	m_DirectInput->Release();
}

// ---------------------------------------------------------------------------

void Input::DetectInput(double dt)
{
	// Store mouse state
	DIMOUSESTATE currentMouseState;
	// Store keyboard state
	BYTE keyboardState[256];

	// Make sure the application has control over the device
	m_pDIKeyboard->Acquire();
	m_pDIMouse->Acquire();

	// Update the state of our devices
	m_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &currentMouseState);
	m_pDIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	if (m_pActiveCamera != nullptr)
	{
		float fUpdateRate = (float)(m_pActiveCamera->GetCameraSpeed() * dt);

		// Check for input
		if ((keyboardState[DIK_LEFT] & 0x80) | (keyboardState[DIK_A] & 0x80))
		{
			m_pActiveCamera->LeftRight() -= fUpdateRate;
		}
		if ((keyboardState[DIK_RIGHT] & 0x80) | (keyboardState[DIK_D] & 0x80))
		{
			m_pActiveCamera->LeftRight() += fUpdateRate;
		}
		if ((keyboardState[DIK_UP] & 0x80) | (keyboardState[DIK_W] & 0x80))
		{
			m_pActiveCamera->BackwardForward() += fUpdateRate;
		}
		if ((keyboardState[DIK_DOWN] & 0x80) | (keyboardState[DIK_S] & 0x80))
		{
			m_pActiveCamera->BackwardForward() -= fUpdateRate;
		}
		if (currentMouseState.lX != m_MouseLastState.lX ||
			currentMouseState.lY != m_MouseLastState.lY)
		{
			m_pActiveCamera->CameraYaw() += (currentMouseState.lX * 0.001f);
			m_pActiveCamera->CameraPitch() += (currentMouseState.lY * 0.001f);

			m_MouseLastState = currentMouseState;
		}

		m_pActiveCamera->UpdateCamera();
	}
}

// ---------------------------------------------------------------------------

bool Input::InitializeDirectInput(HWND hWnd, HINSTANCE hInstance, Camera* camera)
{
	// Create the direct input object
	HR(DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_DirectInput,
		NULL));

	// Create the keyboard input object
	HR(m_DirectInput->CreateDevice(GUID_SysKeyboard,
		&m_pDIKeyboard,
		NULL));

	// Create the mouse input object
	HR(m_DirectInput->CreateDevice(GUID_SysMouse,
		&m_pDIMouse,
		NULL));

	// Set keyboard data format
	HR(m_pDIKeyboard->SetDataFormat(&c_dfDIKeyboard));
	HR(m_pDIKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

	// Set mouse data format
	HR(m_pDIMouse->SetDataFormat(&c_dfDIMouse));
	HR(m_pDIMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY));

	SetActiveCamera(camera);

	return true;
}

// ---------------------------------------------------------------------------

void Input::SetActiveCamera(Camera* camera)
{
	m_pActiveCamera = camera;
}

// ---------------------------------------------------------------------------