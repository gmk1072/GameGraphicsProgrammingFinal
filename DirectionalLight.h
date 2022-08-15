#pragma once
#include <DirectXMath.h>
#include "DirectionalLightLayout.h"
#include "Light.h"

using namespace DirectX;

// Simple class which exposes underlying shader struct values and inherits from
// a common Light class.
class DirectionalLight : private Light
{
	friend class LightRenderer;
public:
	// Exposing struct values publicly
	XMFLOAT4& diffuseColor;
	XMFLOAT4& ambientColor;
	XMFLOAT3& direction;
	float& intensity;

private:
	DirectionalLight();
	~DirectionalLight();

	// Mesh held in LightRenderer
	// PixelShader held in LightRenderer
	// VertexShader held in LighRenderer

	// The layout of the directional light as found in the shader.
	DirectionalLightLayout directionalLightLayout;
};