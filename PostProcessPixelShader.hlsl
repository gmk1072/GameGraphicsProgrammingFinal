//This is a separate shader so that other post processes can be added here without having to go through the blur shaders...maybe
//Could move into vertical blur shader.

Texture2D colorTexture		: register(t0);
Texture2D bloomTexture		: register(t1);
Texture2D glowTexture		: register(t2);
Texture2D volumetricTexture	: register(t3);
SamplerState finalSampler	: register(s0);

struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

float4 main(TargetCoords input) : SV_TARGET
{
	float4 col = colorTexture.Sample(finalSampler, input.uv);
	float4 bloom = bloomTexture.Sample(finalSampler, input.uv);
	float4 volumetricLighting = volumetricTexture.Sample(finalSampler, input.uv);
	float4 glow = glowTexture.Sample(finalSampler, input.uv);
	float4 final = col + bloom + glow + volumetricLighting;
	final.a = 1.0;
	return final;
}