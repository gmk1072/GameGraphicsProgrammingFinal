#include "Vertex.hlsli"

// Texture information for objects
Texture2D albedo			: register(t0);
SamplerState albedoSampler	: register(s0);

// Output our pixel data to render targets
struct PSOutput
{
	float4 color		: SV_Target0;
	float4 worldPos		: SV_Target1;
	float4 normals		: SV_Target2;
	float4 emission		: SV_Target3;
};

// Output all of our data to render targets (screen sized textures)
PSOutput main(VertexToPixel input)
{
	// Redirect interpolated pixels to render targets
	PSOutput output;

	// sample color information
	output.color = albedo.Sample(albedoSampler, input.uv);

	// sample world pos
	output.worldPos = float4(input.worldPos, 1.0f);

	// convert normals to color space
	output.normals = float4((normalize(input.normal) + 1.0f) / 2.0f, 1.0f);

	// set emission to black = 0
	output.emission = float4(0, 0, 0, 1);

	// return to render targets
	return output;
}