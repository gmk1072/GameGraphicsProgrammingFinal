TextureCube Skybox		: register(t0);	// Texture cube used for sky
SamplerState Sampler	: register(s0);	// Sampler used to sample the sky texture

cbuffer externalData : register(b0)
{
	float4 tint;
};

// Input from vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD;	// Texture coordinate past in by vertex shader
}; 


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Samples for the sky texture and applies tint
	return Skybox.Sample(Sampler, input.uvw) * tint;
}