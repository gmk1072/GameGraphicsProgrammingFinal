#include "Vertex.hlsli"

// Texture information for objects
Texture2D albedo			: register(t0);
Texture2D normalMap			: register(t1);
Texture2D emissionMap		: register(t2);
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

	// Normalize
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Sample from normal map
	float4 normalSampled = normalMap.Sample(albedoSampler, input.uv) * 2 - 1;

	// Calculate TBN
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.normal, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// sample color information
	output.color = albedo.Sample(albedoSampler, input.uv);

	// sample world pos
	output.worldPos = float4(input.worldPos, 1.0f);

	// convert normals to color space
	output.normals = float4(normalize(mul(normalSampled.xyz, TBN) + 1.0f) / 2.0f, 1.0f);

	// set emission to black = 0
	output.emission = emissionMap.Sample(albedoSampler, input.uv);

	// return to render targets
	return output;
}