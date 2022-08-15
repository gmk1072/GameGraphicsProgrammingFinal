#ifndef DIRECTIONAL_LIGHT_LAYOUT_H
#define DIRECTIONAL_LIGHT_LAYOUT_H

#include "ShaderTypes.h"

// Struct/cbuffer which holds directional light information.
CBUFFER(DirectionalLightLayout, b0)
{
	float4 diffuse;
	float4 ambient;
	float3 direction;
	float intensity;
};

#endif // !DIRECTIONAL_LIGHT_LAYOUT_H
