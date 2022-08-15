#pragma once
#include "Camera.h"

// Extends camera. Gives orthogonal projection matrix.
// Should be made an entity.
class CameraGame :
	public Camera
{
public:
	CameraGame();
	~CameraGame();

	// Updates the camera with an orthogonal projection matrix.
	void UpdateProjectionMatrix(float aspectRatio);

	// Getters and Setters
	float GetViewHeight();
	void SetViewHeight(float viewHieght);
protected:
	float viewHeight;
};

