#include "Camera.h"

// ----------------------------------------------------------------------------

Camera::Camera(float fNearPlaneWidth,
	float screenNear,
	float screenDepth, 
	int clientWidth, 
	int clientHeight)
{
	// Camera setup
	m_vCameraPosition = Vector4(0.0f, 3.0f, -8.0f, 0.0f);
	m_vCameraTarget = Vector4(0.0f, 0.0f, -1.0f, 0.0f);
	m_vCameraUp = Vector4(0.0f, 1.0f, 0.0f, 0.0f);

	// Create the view matrix
	m_mView = XMMatrixLookAtLH(m_vCameraPosition, m_vCameraTarget, m_vCameraUp);
	// Create the projection matrix
	m_mPerspectiveProjection = XMMatrixPerspectiveLH(fNearPlaneWidth * DirectX::XM_PI,
		(float)clientWidth / clientHeight,
		screenNear,
		screenDepth);

	m_mOrthographicProjection = XMMatrixOrthographicLH((float)clientWidth / 256.0f, 
		(float)clientHeight / 256.0f,
		screenNear, 
		screenDepth);
}

// ----------------------------------------------------------------------------

Camera::~Camera()
{
}

// ----------------------------------------------------------------------------

void Camera::UpdateProjection(float fNearPlaneWidth, 
	float screenNear,
	float screenDepth,
	int clientWidth,
	int clientHeight)
{
	// Create the projection matrix
	m_mPerspectiveProjection = XMMatrixPerspectiveLH(fNearPlaneWidth * DirectX::XM_PI,
		(float)clientWidth / clientHeight,
		screenNear,
		screenDepth);

	// Create the orthographic projection matrix
	m_mOrthographicProjection = XMMatrixOrthographicLH((float)clientWidth,
		(float)clientHeight,
		screenNear,
		screenDepth);
}

// ----------------------------------------------------------------------------

void Camera::UpdateCamera()
{
	// Create the camera rotation matrix
	m_mCameraRotationMatrix = Matrix::CreateFromYawPitchRoll(m_fCameraYaw, m_fCameraPitch, 0.0f);
	// Transform the camera target vector using the rotation matrix
	m_vCameraTarget = Vector4::Transform(m_vDefaultForward, m_mCameraRotationMatrix);
	m_vCameraTarget.Normalize();

	// Calculate the rotation matrix around Y axis
	Matrix matYRotationMatrix = Matrix::CreateRotationY(m_fCameraYaw);
	// Rotate the right, up and forward vector of the camera using the Y rot matrix
	m_vCameraUp = Vector4::Transform(m_vCameraUp, matYRotationMatrix);
	m_vCameraRight = Vector4::Transform(m_vDefaultRight, matYRotationMatrix);
	m_vCameraForward = Vector4::Transform(m_vDefaultForward, matYRotationMatrix * Matrix::CreateRotationX(m_fCameraPitch));

	// Update camera position
	m_vCameraPosition += m_fMoveLeftRight * m_vCameraRight;
	m_vCameraPosition += m_fMoveBackForward * m_vCameraForward;

	m_fMoveLeftRight = 0.0f;
	m_fMoveBackForward = 0.0f;

	// Update camera target
	m_vCameraTarget = m_vCameraPosition + m_vCameraTarget;

	// Update view matrix
	m_mView = XMMatrixLookAtLH(m_vCameraPosition, m_vCameraTarget, m_vCameraUp);
}

// ----------------------------------------------------------------------------