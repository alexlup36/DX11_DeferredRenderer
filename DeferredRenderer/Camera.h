#ifndef CAMERA_H
#define CAMERA_H

#include <d3d11.h>
#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
friend class Input;

public:
	Camera(float fNearPlaneWidth, 
		float screenNear,
		float screenDepth,
		int clientWidth,
		int clientHeight);
	~Camera();

	void UpdateProjection(float fNearPlaneWidth,
		float screenNear,
		float screenDepth,
		int clientWidth,
		int clientHeight);
	void UpdateCamera();
	inline const Matrix& View() { return m_mView; }
	inline const Matrix& PerpectiveProjection() { return m_mPerspectiveProjection; }
	inline const Matrix& OrthographicProjection() { return m_mOrthographicProjection; }

	inline Vector4 Position() { return m_vCameraPosition; }
	inline void SetPosition(const Vector4& newPosition) { m_vCameraPosition = newPosition; }

	inline float GetCameraSpeed() { return m_fCameraSpeed; }

protected:

	inline float& LeftRight()			{ return m_fMoveLeftRight; }
	inline float& BackwardForward()		{ return m_fMoveBackForward; }
	inline float& CameraYaw()			{ return m_fCameraYaw; }
	inline float& CameraPitch()			{ return m_fCameraPitch; }

private:

	Matrix m_mView;
	Matrix m_mPerspectiveProjection;
	Matrix m_mOrthographicProjection;

	Matrix m_mCameraRotationMatrix;
	 
	Vector4 m_vDefaultForward	= Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 m_vDefaultRight		= Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 m_vCameraForward	= Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 m_vCameraRight		= Vector4(1.0f, 0.0f, 0.0f, 0.0f);

	Vector4 m_vCameraPosition;
	Vector4 m_vCameraTarget;
	Vector4 m_vCameraUp;

	float m_fCameraSpeed = 15.0f;

	float m_fMoveLeftRight		= 0.0f;
	float m_fMoveBackForward	= 0.0f;

	float m_fCameraYaw		= 0.0f;
	float m_fCameraPitch	= 0.0f;
};

#endif // CAMERA_H