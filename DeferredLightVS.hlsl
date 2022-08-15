// Basic vertex shader that forwards model verts to a pixel shader which will
// be used to determine which pixels to light.
#include "Vertex.hlsli"

cbuffer externalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

struct DLVStoPS
{
    float4 position : SV_Position;
    float3 worldPos : POSITION;
};

DLVStoPS main(VertexShaderInput input)
{
    DLVStoPS output;
    matrix worldViewProj = mul(mul(world, view), projection);
    output.position = mul(float4(input.position, 1.0f), worldViewProj);
    output.worldPos = (float3) mul(float4(input.position, 1.0f), world);
    return output;
}