#include "PointLightLayout.h"

// Lighting information to sample from
Texture2D worldPosTexture : register(t0);
Texture2D normalsTexture : register(t1);
SamplerState deferredSampler : register(s0);

cbuffer screenInfo : register(b1)
{
    float2 screenSize;
}

struct DLVStoPS
{
    float4 position : SV_Position;
    float3 worldPos : POSITION;
};

float4 main(DLVStoPS input) : SV_TARGET
{
    // Sample based on vertex xy in screen space
    float2 uv = input.position.xy / screenSize;
    float3 pos = worldPosTexture.Sample(deferredSampler, uv).xyz;

    // See if we should discard
    float3 centerToPos = position - pos;
    float distSq = dot(centerToPos, centerToPos);
    clip((radius * radius) - distSq);

	// revert normals to -1 to 1
    float3 n = (normalsTexture.Sample(deferredSampler, uv).xyz * 2.0f) - 1.0f;

    // dir from object to light
    float3 dir = normalize(centerToPos);

    // normal dot light direction
    float lightAmt = saturate(dot(n, dir));

    // attenuation
    float att = 1.0f / (attConstant + attLinear * sqrt(distSq) + attQuadratic * distSq);
    att = (att - cutoff) / (1 - cutoff);
    att = max(att, 0);

    // add on
    return diffuse * att * lightAmt;
}