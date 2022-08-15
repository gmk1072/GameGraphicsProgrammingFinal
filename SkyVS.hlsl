// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

// Struct sent to vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD; // Direction for cubemap sampling in PS
};


VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// Removes position from view matrix
	matrix viewNoMovement = view;
	viewNoMovement._41 = 0;
	viewNoMovement._42 = 0;
	viewNoMovement._43 = 0;

	// Calculate output position
	matrix viewProj = mul(viewNoMovement, projection);
	float scale = 10.0f;	// Makes sure the model is big enough.
	output.position = mul(float4(input.position * scale, 1.0f), viewProj);	// Find position 

	// To ensure we're at MAX DEPTH, set the Z to the W
	output.position.z = output.position.w;

	// The UVW represents position in the cube from the center
	output.uvw = input.position;

	return output;
}