#include "ParticleLayout.h"

#define ALPHA_CUTOFF 0.1f

// Output our pixel data to render targets
struct PSOutput
{
    float4 color : SV_Target0;
    float4 worldPos : SV_Target1;
    float4 normals : SV_Target2;
    float4 emission : SV_Target3;
};

// Texture information
Texture2D albedo : register(t0);
SamplerState albedoSampler : register(s0);

// Output all of our data to render targets (screen sized textures)
PSOutput main(ParticleVertexToPixel input)
{
	// Redirect interpolated pixels to render targets
    PSOutput output;

	// sample color information
    float4 textColor = albedo.Sample(albedoSampler, input.uv);
    clip(textColor.a - ALPHA_CUTOFF); // Alpha cutout!
    output.color = float4((textColor * input.tint * (textColor.a / ALPHA_CUTOFF)).rgb, 1.0f); //float4(textColor.rgb * lerp(input.tint.rgb, float3(1, 1, 1), textColor.a), 1.0f);

	// sample world pos
    output.worldPos = float4(input.worldPos, 1.0f);

	// convert normals to color space
    output.normals = float4(0.5f, 0.5f, 0.0, 1.0f);

	// set emission to black = 0
    output.emission = float4((textColor * input.tint * (textColor.a / ALPHA_CUTOFF)).rgb, 1.0f); //float4((textColor * input.tint * (textColor.a / ALPHA_CUTOFF)).rgb, 1.0f);

	// return to render targets
    return output;
}