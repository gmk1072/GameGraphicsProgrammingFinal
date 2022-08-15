#include "Camera.h"
#include "MemoryDebug.h"

// --------------------------------------------------------
// Constructor
//
// Creates a camera starting at 0, 0, 0 with no rotation.
// --------------------------------------------------------
Camera::Camera() :
	transform(),
	angleX(0.0f),
	angleY(0.0f)
{
	// Initialize view matrix to current position
	CalculateViewMatrix();
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
Camera::~Camera()
{
}

// --------------------------------------------------------
// Run this every time an update for the camera is required
//
// deltaTime - Amount of time taken to render a frame
// totalTime - Total time since the program started
// --------------------------------------------------------
void Camera::Update(float deltaTime, float totalTime)
{

	/*
	We want this to ONLY UPDATE when:
		- Position has changed OR
		- Rotation has changed
		- how can we do this ?
	*/
	CalculateViewMatrix();
}

// --------------------------------------------------------
// Rotate the camera by some change in angle about the X
// Y axis.
//
// NOTE: This is not additive to the cameras current
// rotation, it WILL be overwritten with a new Pitch and
// Yaw
//
// dx - Angle in radians the camera should rotate about the
//		Y axis
// dy - Angle in radians the camera should rotate about the
//		X axis
// --------------------------------------------------------
void Camera::RotateBy(float dx, float dy)
{
	// Clamp our up and down to 90 degrees
	if (fabsf(angleY + dy) <= XM_PIDIV2)
		angleY += dy;
	
	// Reset angleX to 0ish so we dont lose precision
	if (fabsf(angleX += dx) >= XM_2PI)
		angleX = dx;

	// Calc new quat and set
	XMFLOAT4 newRotation;
	XMStoreFloat4(&newRotation, XMQuaternionRotationRollPitchYaw(angleY, angleX, 0.0f));
	transform.SetRotation(newRotation);
}

// --------------------------------------------------------
// Update the projection matrix to match the given aspect
// ratio.
//
// aspectRatio - The new aspect ratio
// --------------------------------------------------------
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	// Update our projection matrix for whatever reason
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		aspectRatio,			// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

// --------------------------------------------------------
// Get the view matrix for this camera
// NOTE: View matrix will only update every time Update is
// called!
// --------------------------------------------------------
const XMFLOAT4X4& Camera::GetViewMatrix() const
{
	return view;
}

// --------------------------------------------------------
// Get the projection matrix for this camera
// --------------------------------------------------------
const XMFLOAT4X4& Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

// --------------------------------------------------------
// Calculate this camera's view matrix according to position
// and direction vectors found in Transform.
// --------------------------------------------------------
void inline Camera::CalculateViewMatrix()
{
	// Grab position, forward and up
	XMVECTOR pos = XMLoadFloat3(transform.GetPosition());
	XMVECTOR fwd = XMLoadFloat3(transform.GetForward());
	XMVECTOR up = XMLoadFloat3(transform.GetUp());

	// Calculate and store view, TRANSPOSE FOR HLSL!!!!!
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMMatrixLookToLH(pos, fwd, up)));
}
