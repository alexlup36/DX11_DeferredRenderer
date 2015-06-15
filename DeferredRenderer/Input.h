#ifndef INPUT_H
#define INPUT_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "DXUtil.h"
#include "Camera.h"

class Input
{
public:

	static Input& GetInstance()
	{
		static Input instance;
		return instance;
	}

	void DetectInput(double dt);
	bool InitializeDirectInput(HWND hWnd, HINSTANCE hInstance, Camera* camera);

	void SetActiveCamera(Camera* camera);

private:

	Input() 
	{
		m_pActiveCamera = nullptr;
		m_pDIKeyboard	= nullptr;
		m_pDIMouse		= nullptr;
	};
	~Input();

	Input(Input const&)				= delete;
	void operator=(Input const&)	= delete;

	// Private members
	DIMOUSESTATE				m_MouseLastState;
	LPDIRECTINPUT8				m_DirectInput;
	IDirectInputDevice8*		m_pDIKeyboard;
	IDirectInputDevice8*		m_pDIMouse;

	Camera*						m_pActiveCamera;
};

#endif // INPUT_H