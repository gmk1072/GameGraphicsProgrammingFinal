#pragma once

#include "Transform.h"
#include "PointLightLayout.h"
#include "Light.h"

// Enum which defines different attenuation modes for a point light.
enum class PointLightAttenuation
{
	CUSTOM,					// Provide custom radius and attenuation values
	ATTEN_FROM_RADIUS,		// Calculate attenuation values from a given radius
	RADIUS_FROM_ATTEN		// Calculate radius from given attenuation values
};

// Simple class which exposes underlying shader struct values and inherits from
// a common Light class.
class PointLight : private Light
{
	friend class LightRenderer;
public:
	// Exposing struct information publicly
	XMFLOAT4& diffuseColor;
	XMFLOAT3& position;
	float& radius;
	float& atenConstant;
	float& atenLinear;
	float& atenQuadratic;

	PointLightAttenuation attenuationType;
private:
	PointLight(PointLightAttenuation attenuationType);
	~PointLight();

	// Attenuation specific functions
	void CalcRadiusFromAtten();
	void CalcAttenFromRadius();
	void PrepareForShader();

	// Mesh held in LightRenderer
	// PixelShader held in LightRenderer
	// VertexShader held in LighRenderer
	Transform transform;
	PointLightLayout pointLightLayout;
};

