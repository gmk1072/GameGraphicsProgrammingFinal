#include "PointLight.h"


// --------------------------------------------------------
// Constructor
//
// Creates a PointLightLayout struct within this class
// internally and prepares to calculate radius/attenuation
// based on provided attenuation type.
//
// attenuationType - The type of attenuation this point light
//					 should use.
// --------------------------------------------------------
PointLight::PointLight(PointLightAttenuation attenuationType) :
	pointLightLayout(),
	diffuseColor(pointLightLayout.diffuse),
	position(pointLightLayout.position),
	radius(pointLightLayout.radius),
	atenConstant(pointLightLayout.attConstant),
	atenLinear(pointLightLayout.attLinear),
	atenQuadratic(pointLightLayout.attQuadratic),
	attenuationType(attenuationType)
	
{
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
PointLight::~PointLight()
{
}

// --------------------------------------------------------
// Calculates a radius from given attenuation values. Visit
// the link below to learn how this equation was derived.
// --------------------------------------------------------
void PointLight::CalcRadiusFromAtten()
{
	// set cutoff back to 0
	pointLightLayout.cutoff = 0;

	//https://learnopengl.com/#!Advanced-Lighting/Deferred-Shading
	// Calculate radius based on attenuation
	float lightMax = XMMax<float>(
		XMMax<float>(diffuseColor.x, diffuseColor.y), diffuseColor.z
		);
	pointLightLayout.radius = (-atenLinear + sqrtf(atenLinear * atenLinear - 4 * atenQuadratic * (atenConstant - (256.0f / 2.0f) * lightMax))) / (2.0f * atenQuadratic);
}

// --------------------------------------------------------
// Calculates attenuation values based on a given radius.
// Visit the link below to learn how this equation was derived.
// https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
// --------------------------------------------------------
void PointLight::CalcAttenFromRadius()
{
	float lightMax = XMMax<float>(
		XMMax<float>(diffuseColor.x, diffuseColor.y), diffuseColor.z
		);
	float cutoff = lightMax / 4.0f;
	pointLightLayout.attConstant = 1.0f;
	pointLightLayout.attLinear = 2.0f / radius;
	pointLightLayout.attQuadratic = 1.0f / (radius * radius);
	pointLightLayout.cutoff = cutoff * 2;
}

// --------------------------------------------------------
// Calls associated attenuation & radius calculation functions
// according to this point lights attenuation type.
// 
// NOTE: Always call this before uploading PointLightLayout
// structure to gfx.
// --------------------------------------------------------
void PointLight::PrepareForShader()
{
	switch (attenuationType)
	{
		case PointLightAttenuation::CUSTOM:
			break;
		case PointLightAttenuation::ATTEN_FROM_RADIUS:
			CalcAttenFromRadius();
			break;
		case PointLightAttenuation::RADIUS_FROM_ATTEN:
			CalcRadiusFromAtten();
			break;
		default:
			break;
	}

	// Set scale and position for world matrix
	transform.SetPosition(position);
	transform.SetScale(radius, radius, radius);
}
