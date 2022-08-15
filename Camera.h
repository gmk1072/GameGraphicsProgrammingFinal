#pragma once

#include "Transform.h"

// THIS SHOULD BE CONSIDERED AN ENTITY.
class Camera
{
public:
	// Empty constructor, will initialize to 0,0,0 and face forward
	Camera();
	~Camera();
	
	// Update this camera
	virtual void Update(float deltaTime, float totalTime);

	// Update projection matrix when ever aspect ratio wants to be changed
	virtual void UpdateProjectionMatrix(float aspectRatio);

	// ROtate this camera by some amount on x and y
	void RotateBy(float dx, float dy);

	// Get the current view matrix of this camera
	const XMFLOAT4X4& GetViewMatrix() const;
	const XMFLOAT4X4& GetProjectionMatrix() const;

	// Camera transform for location in space
	// Also contains right, forward, up vector information
	Transform transform;

protected:
	// Calculate the view matrix of this camera
	void CalculateViewMatrix();

	// The matrices to go from model space to screen space
	XMFLOAT4X4 view;
	XMFLOAT4X4 projectionMatrix;

	// Angle in X and Y axises
	float angleX;
	float angleY;
};

