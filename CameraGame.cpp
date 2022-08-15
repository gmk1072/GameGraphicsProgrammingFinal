#include "CameraGame.h"
#include "MemoryDebug.h"

CameraGame::CameraGame()
{
	viewHeight = 4.0f;
}


CameraGame::~CameraGame()
{
}

void CameraGame::UpdateProjectionMatrix(float aspectRatio)
{
	// Update our projection matrix for whatever reason
	XMMATRIX P = XMMatrixOrthographicLH(
		aspectRatio * viewHeight,	// View Width
		viewHeight,					// View Height
		0.01f,						// Near Plane
		1000.0f						// Far Plane
		);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

float CameraGame::GetViewHeight()
{
	return this->viewHeight;
}

void CameraGame::SetViewHeight(float viewHieght)
{
	this->viewHeight = viewHeight;
}
