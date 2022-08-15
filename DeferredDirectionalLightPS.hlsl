// Include the directional light layout
#include "DirectionalLightLayout.h"

// Lighting information to sample from
Texture2D worldPosTexture : register(t0);
Texture2D normalsTexture : register(t1);
SamplerState deferredSampler : register(s0);

// Input will actually be from the fullscreen quad shader
struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

float4 main(TargetCoords input) : SV_TARGET
{
    // Sample based on vertex xy in screen space
    float3 pos = worldPosTexture.Sample(deferredSampler, input.uv).xyz;

	// revert normals to -1 to 1
    float3 n = (normalsTexture.Sample(deferredSampler, input.uv).xyz * 2.0f) - 1.0f;

    // dir from object to light
    float3 dir = normalize(direction);

    // normal dot light direction
    float lightAmt = saturate(dot(n, -dir));

    // add on
    return ((diffuse * lightAmt) + ambient); //* col;
}