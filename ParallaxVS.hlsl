#include "Vertex.hlsli"

cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix inverseTransposeWorld;
	float3 viewPos;
};

struct PVStoPS
{
	float4 position : SV_Position;
	float3 worldPos : POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD;
	float3 viewTan		: VIEWTANGENT;
	float3 posTan		: POSITIONTANGENT;
};

PVStoPS main(VertexShaderInput input)
{
	PVStoPS output;

	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.worldPos = (float3) mul(float4(input.position, 1.0f), world);

	output.normal = mul(input.normal, (float3x3)inverseTransposeWorld);
	output.tangent = mul(input.tangent, (float3x3)inverseTransposeWorld);

	float3 N = mul(input.normal, (float3x3)inverseTransposeWorld);
	float3 T = mul(input.tangent, (float3x3)inverseTransposeWorld);
	//float3 B = cross(T, N);
	//float3 N = output.normal;
	//loat3 T = normalize(output.tangent - N * dot(output.normal, N));
	float3 B = cross(T, N);
	float3x3 TBN = transpose(float3x3(T, B, N));

	output.viewTan = mul(viewPos, TBN);
	output.posTan = mul(output.worldPos, TBN);

	// Interpolate UV coordinates
	output.uv = input.uv;

	return output;
}