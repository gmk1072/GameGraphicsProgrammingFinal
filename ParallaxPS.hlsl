
// Texture information for objects
Texture2D albedo			: register(t0);
Texture2D normalMap			: register(t1);
Texture2D depthMap				: register(t2);
SamplerState albedoSampler	: register(s0);

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

struct PSOutput
{
	float4 color		: SV_Target0;
	float4 worldPos		: SV_Target1;
	float4 normals		: SV_Target2;
	float4 emission		: SV_Target3;
};

float2 ParallaxMapping(float2 texCoords, float3 viewDir)
{
	float height_scale = 0.1f;
	float height = 1-depthMap.Sample(albedoSampler, texCoords).r;
	float2 p = viewDir.xy / viewDir.z * (height * height_scale);
	return texCoords - p;
};

PSOutput main(PVStoPS input) : SV_TARGET
{
	PSOutput output;

	float3 viewDir = normalize(input.viewTan - input.posTan);
	float2 texCoords = ParallaxMapping(input.uv, viewDir);
	//output.color = albedo.Sample(albedoSampler, input.uv);
	//if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
		//discard;

	// sample color information
	output.color = albedo.Sample(albedoSampler, texCoords);
	//output.color = float4(1, 0, 0, 1);

	// sample world pos
	output.worldPos = float4(input.worldPos, 1.0f);

	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.normal, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);
	// convert normals to color space
	float4 normalSampled = normalMap.Sample(albedoSampler, texCoords) * 2 - 1;
	output.normals = float4(normalize(mul(normalSampled.xyz, TBN) + 1.0f) / 2.0f, 1.0f);

	// set emission to black = 0
	output.emission = float4(0, 0, 0, 1);

	return output;
}