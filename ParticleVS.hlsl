// ALL BASED ON http://twvideo01.ubm-us.net/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf
// WITH HELP FROM CHRIS Cascioli
// RESET COUNTERS EACH DRAW HERE https://www.gamedev.net/topic/662293-reset-hidden-counter-of-structuredappendconsume-buffers/
// CLEAR ALIVE LIST EACH DRAW
//#include "Particle.hlsli"
#include "Vertex.hlsli"
#include "ParticleLayout.h"
#include "ShaderConstants.h"

// Constant UV information that will never change
static float2 uv[4] =
{
        float2(1.0f, 0.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
};

// World information
cbuffer externalData : register(b0)
{
    matrix view; // 64
    matrix projection; // 64
};

uint GetTextureOffset(uint flags)
{
    // move flags right 12 and return
    return (flags >> 12);
}

// Return 0 if the flag is not contained, otherwise, return 1
uint isFlagContained(uint theFlag, uint allFlags)
{
    return ((allFlags & theFlag) == theFlag);
}

// Particle information to take from
StructuredBuffer<Particle> particlePool : register(t0); // All particles
StructuredBuffer<ParticleAlive> aliveList : register(t1); // Alive particles (will be used for drawing)

ParticleVertexToPixel main(uint v_id : SV_VertexID, uint i_id : SV_InstanceID)
{
    // Grab current particle
    Particle particle = particlePool[aliveList[i_id].index];

    // Per instance index counter
    uint particleVertexIndex = v_id % 4;    // The index of the vertex of this particle

    // Store flags for future use
    uint pFlags = particle.flags;

    // Interp value to be reused
    float interpVal = 1.0f - (particle.currAge / particle.maxAge);

    // Interp size inforation
    float2 size = lerp(particle.sSize, particle.eSize, interpVal * isFlagContained(INTERP_SIZE, pFlags));

	// When billboarding, normal always facing directly towards camera
	// Obtain view matrix Up, Right and Normal
    float3 halfRight = normalize(view._11_21_31) * size.x * 0.5f;
    float3 halfUp = normalize(view._12_22_32) * size.y * 0.5f;
	const float3 verts[4] =
	{
        halfRight - halfUp,
		-halfRight - halfUp,
        halfRight + halfUp,
		-halfRight + halfUp
		
	};

    // Get world matrix for a single particle (particle center being at world pos)
    const matrix world =
    {
        { 1, 0, 0, 0},
        { 0, 1, 0, 0},
        { 0, 0, 1, 0},
        float4(particle.worldPos, 1.0f)
    };

    // Calc WVP
    matrix worldViewProj = mul(mul(world, view), projection);

	// Get ready to output to PS
    ParticleVertexToPixel output;

    // Particle specific information
    output.age = particle.currAge;
    output.tint.rgb = lerp(particle.sTint, particle.eTint, interpVal * isFlagContained(INTERP_TINT, pFlags));
    output.tint.a = lerp(particle.sAlpha, particle.eAlpha, interpVal * isFlagContained(INTERP_ALPHA, pFlags));

    // Positional information
	// Calculate WVP for each particle
	// Because normal always faces camera, its just the opposite of the forward. 
    output.worldPos = (float3) mul(float4(verts[particleVertexIndex], 1.0f), world);
    output.position = mul(float4(verts[particleVertexIndex], 1.0f), worldViewProj);
    output.uv = uv[particleVertexIndex] * float2(1.0f / NUM_TEXTURES_IN_ATLAS, 1.0f); // replace .5 with 1 / numOfTexts
    output.uv.x += GetTextureOffset(pFlags) * (1.0f / NUM_TEXTURES_IN_ATLAS); // replace .5 with 1 / numOfTexts 
    // -------------
    // 0 - 0, .5 | 0 .5
    // 1 - .5, 1 | 0 .5


	// Move on to pixel shader (deferred pixel shader)
	return output;
}