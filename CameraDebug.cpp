#include "CameraDebug.h"
#include "MemoryDebug.h"

CameraDebug::CameraDebug()
{
}


CameraDebug::~CameraDebug()
{
}

void CameraDebug::Update(float deltaTime, float totalTime)
{
	// Move this camera around
	if (GetAsyncKeyState('W') & 0x8000)
		transform.MoveForward(0.0f, 0.0f, 2.0f * deltaTime);
	if (GetAsyncKeyState('S') & 0x8000)
		transform.MoveForward(0.0f, 0.0f, -2.0f * deltaTime);
	if (GetAsyncKeyState('D') & 0x8000)
		transform.MoveForward(2.0f * deltaTime, 0.0f, 0.0f);
	if (GetAsyncKeyState('A') & 0x8000)
		transform.MoveForward(-2.0f * deltaTime, 0.0f, 0.0f);
	if (GetAsyncKeyState(' ') & 0x8000)
		transform.Move(0.0f, 2.0f * deltaTime, 0.0f);
	if (GetAsyncKeyState('X') & 0x8000)
		transform.Move(0.0f, -2.0f * deltaTime, 0.0f);

	CalculateViewMatrix();
}

