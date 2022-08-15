struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

// Takes in a vertex ID and converts it into a vertex of a full screen quad.
// This is used so that we can have our pixel shader sample our textures in
// our deferred shader according to the size of the screen.
TargetCoords main(uint id: SV_VertexID)
{
	TargetCoords output;

	// 1.0
	// ^
	// i
	// 0.0 -> 1.0
	output.uv = float2((id << 1) & 2, id & 2);
	output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);

	return output;
}