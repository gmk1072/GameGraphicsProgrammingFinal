#ifndef LIGHTS_H
#define LIGHTS_H
#include "ShaderTypes.h"

// -- OBSOLETE HEADER FILE, SHOULD NOT BE USED ANYWHERE NOW -- 

// --------------------------------------------------------
// Directional Light
//
// Global light that illuminates all objects
// --------------------------------------------------------
struct DirectionalLight_old
{
	//DirectX::XMFLOAT4 AmbientColor;
	float4 DiffuseColor; // 16
	float3 Direction; // 12
	float Intensity; // 4
	//unsigned int : 4; // padding for arrays of lights
};


// --------------------------------------------------------
// Point Light
//
// Light that lives at a point in space, will never dissapate
// --------------------------------------------------------
struct PointLight_old
{
	float4 DiffuseColor; // 16
	float3 Position; // 12
	float Intensity; // 4
};


// --------------------------------------------------------
// Spot Light
//
// Light that shines in a cone
// --------------------------------------------------------
struct SpotLight
{
	float4 DiffuseColor; // 16
	float3 Direction; // 12
	float Intensity; // 4
	float3 Position; // 12
	float Angle; // 4
};

#endif