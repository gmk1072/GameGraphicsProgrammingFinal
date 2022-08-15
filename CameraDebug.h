#pragma once
#include "Camera.h"

// Extends Camera. Is movable.
// Should be made an entity.
class CameraDebug :
	public Camera
{
public:
	CameraDebug();
	~CameraDebug();

	// Updates the camera with movement controls
	void Update(float deltaTime, float totalTime);
};

