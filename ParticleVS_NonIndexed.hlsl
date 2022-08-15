// ALL BASED ON http://twvideo01.ubm-us.net/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf
// WITH HELP FROM CHRIS Cascioli
// RESET COUNTERS EACH DRAW HERE https://www.gamedev.net/topic/662293-reset-hidden-counter-of-structuredappendconsume-buffers/
// CLEAR ALIVE LIST EACH DRAW
#include "Particle.hlsli"
#include "Vertex.hlsli"

// Constant UV information that will never change
static float2 uv[4] =
{
        float2(1.0f, 0.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
};

// Constant indexing information that will never change
static float index[6] = { 0, 1, 2, 2, 1, 3 };

// World information
cbuffer externalData : register(b0)
{
    matrix view; // 64
    matrix projection; // 64
};

// Particle information to take from
StructuredBuffer<Particle> particlePool : register(t0); // All particles
StructuredBuffer<ParticleAlive> aliveList : register(t1); // Alive particles (will be used for drawing)

VertexToPixel main(uint i_id : SV_InstanceID, uint v_id : SV_VertexID)
{
    // Per instance index counter
    uint particleVertexID = v_id % 6;

	// When billboarding, normal always facing directly towards camera
	// Obtain view matrix Up, Right and Normal
	float3 halfRight = normalize(view._11_21_31) * 0.2f;
    float3 halfUp = normalize(view._12_22_32) * 0.2f;
	float3 n = normalize(view._13_23_33);
	float3 verts[4] =
	{
        halfRight - halfUp,
		-halfRight - halfUp,
        halfRight + halfUp,
		-halfRight + halfUp
		
	};

	// Grab current particle
	Particle particle = particlePool[aliveList[i_id].index];

    // Get world matrix for a single particle (particle center being at world pos)
    matrix world =
    {
        { 1, 0, 0, 0},
        { 0, 1, 0, 0},
        { 0, 0, 1, 0},
        float4(particle.worldPos, 1.0f)
        //{ particle.worldPos.x, particle.worldPos.y, particle.worldPos.z, 1 },
    };

    // Calc WVP
    matrix worldViewProj = mul(mul(world, view), projection);

	// Get ready to output to PS
	VertexToPixel output;

	// Calculate WVP for each particle
	// Because normal always faces camera, its just the opposite of the forward. 
    output.normal = -n; // float3(0, 0, -1);
	output.tangent = halfRight * 2.0f;
    output.worldPos = (float3) mul(float4(verts[index[particleVertexID]], 1.0f), world);
    output.position = mul(float4(verts[index[particleVertexID]], 1.0f), worldViewProj);
    output.uv = uv[index[particleVertexID]];

	// Move on to pixel shader (deferred pixel shader)
	return output;
}