#include "DirectionalLight.h"

// --------------------------------------------------------
// Initialize directional light layout struct and connect
// references to the private struct.
// --------------------------------------------------------
DirectionalLight::DirectionalLight() :
	directionalLightLayout(),
	diffuseColor(directionalLightLayout.diffuse),
	ambientColor(directionalLightLayout.ambient),
	direction(directionalLightLayout.direction),
	intensity(directionalLightLayout.intensity)
{
	
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
DirectionalLight::~DirectionalLight()
{
}