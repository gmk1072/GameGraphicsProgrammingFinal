#ifndef POINT_LIGHT_LAYOUT
#define POINT_LIGHT_LAYOUT

#include "ShaderTypes.h"

// Struct/cbuffer which holds point light information.
CBUFFER(PointLightLayout, b0)
{
	// Light specific
	float4 diffuse; // 16
	float3 position; // 12 (16)
	float radius;

	// Attenuation information
	float attConstant;
	float attLinear;
	float attQuadratic;
	float cutoff;
};

#endif // !POINT_LIGHT_LAYOUT

