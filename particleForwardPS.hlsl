#include "ParticleLayout.h"

// Texture information
Texture2D albedo : register(t0);
SamplerState albedoSampler : register(s0);

// Output all of our data to render targets (screen sized textures)
float4 main(ParticleVertexToPixel input) : SV_Target
{
	// sample color information
    float4 textColor = albedo.Sample(albedoSampler, input.uv) * input.tint;

	// return to back buffer
    return textColor;
}