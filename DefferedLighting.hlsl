// Ignore loop unroll warning
#pragma warning(disable: 3557)

// Light limits
#include "ShaderConstants.h"
#include "Lights.h"

// Lighting information to sample from
Texture2D colorTexture			: register(t0);
Texture2D worldPosTexture		: register(t1);
Texture2D normalsTexture		: register(t2);
Texture2D emissionTexture		: register(t3);
SamplerState deferredSampler	: register(s0);

// Light data for all lights
// Global information independent of current object
cbuffer LightData : register(b0)
{
	// Matching struct definition from C++ for dirlight
	DirectionalLight_old directionalLights[MAX_DIR_LIGHTS];
	PointLight_old pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];

	// global light information
	// Ambient color used for entire scene
	float4 AmbientColor;

	//Bloom pixel threshold -- I see no reason to make it in a buffer of its own
	float ColorThreshold;

	//Glow for emission
	float GlowPercentage;
};

// Input info from vertex shader
struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

// Output info for color with light and pixels to blur
struct DefferedOut {
	float4 color		: SV_Target0;
	float4 bloom		: SV_Target1;
	float4 glow			: SV_Target2;
};


DefferedOut main(TargetCoords input)
{
	DefferedOut output;

	// Sample color, world pos, normals and emissions
	float4 emission = emissionTexture.Sample(deferredSampler, input.uv);
	//this feels gross for some reason
	if (length(emission.xyz) != 0) {
		output.color = emission;
		output.bloom = emission * ColorThreshold;
		output.glow = emission * GlowPercentage;
		return output;
	}

	float4 col = colorTexture.Sample(deferredSampler, input.uv);
	float3 pos = worldPosTexture.Sample(deferredSampler, input.uv).xyz;

	// revert normals to -1 to 1
	float3 n = (normalsTexture.Sample(deferredSampler, input.uv).xyz * 2.0f) - 1.0f;

	// Perform lighting calc and return to light texture
	// Resultant light, tmp dir and lightamt
	float4 totalLight = 0;
	float3 dir;
	float lightAmt;
	int i;

	// calc dir light
	for (i = MAX_DIR_LIGHTS; i--;)
	{
		// Calculate direction from object to light
		// This will just be the negated normalized direction of the light since
		// the light is essentially hitting the same spot on the object all the time
		// Negated late for non-negative dotproduct result coming up
		dir = normalize(directionalLights[i].Direction);

		// Calculate how much light is hitting our surface
		// N dot L, normal of surface dot with direction to light from obj
		// clamp amount from 0 to 1 so when angle is > 90, the light amt is
		// 0 instead of negative
		lightAmt = saturate(dot(n, -dir));

		//return directionalLight[i].DiffuseColor * lightAmt;
		totalLight += directionalLights[i].DiffuseColor *
			directionalLights[i].Intensity *
			lightAmt;
	}

	// calc point lights
	for (i = MAX_POINT_LIGHTS; i--;)
	{
		// dir from object to light
		dir = normalize(pointLights[i].Position - pos);

		// normal dot light direction
		lightAmt = saturate(dot(n, dir));

		// add on
		totalLight += pointLights[i].DiffuseColor *
			pointLights[i].Intensity *
			lightAmt;
	}

	// calc spot lights
	for (i = MAX_SPOT_LIGHTS; i--;)
	{
		// dir from object to light
		dir = normalize(spotLights[i].Position - pos);

		// normal dot light direction subtract from angle
		// only accept light from the spotlights direction
		lightAmt = saturate(dot(n, -normalize(spotLights[i].Direction))) *
			saturate(dot(n, dir) - spotLights[i].Angle);

		// add on
		totalLight += spotLights[i].DiffuseColor *
			spotLights[i].Intensity *
			lightAmt;
	}

	// Scale diffuse by how much light is hitting surface
	// Add in ambient color
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(pos, 1.0f);
	output.color = (totalLight + AmbientColor) * col;
	output.bloom = output.color * ColorThreshold;
	output.glow = float4(0, 0, 0, 0);
	return output;
}