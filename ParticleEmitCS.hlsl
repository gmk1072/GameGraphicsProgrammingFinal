// WITH HELP FROM CHRIS Cascioli
// RESET COUNTERS EACH DRAW HERE https://www.gamedev.net/topic/662293-reset-hidden-counter-of-structuredappendconsume-buffers/
// CLEAR ALIVE LIST EACH DRAW
#include "ShaderConstants.h"
#include "ParticleLayout.h"
#include "EmitterLayout.h"

// https://www.shadertoy.com/view/4djSRW
#define HASHSCALE4 float4(1031, .1030, .0973, .1099)
float4 hash41(float p)
{
    float4 p4 = frac(float4(p,p,p,p) * HASHSCALE4);
    p4 += dot(p4, p4.wzxy + 19.19);
    return frac((p4.xxyz + p4.yzzw) * p4.zywx);
}

// Return 0 if the flag is not contained, otherwise, return 1
uint isFlagContained(uint theFlag, uint allFlags)
{
    return ((allFlags & theFlag) == theFlag);
}

/*
    Return a random float in a given range. If the randType &'d with flags
    evals to 0, then the min value is ALWAYS returned.
*/
float randRangeFloat(float randNum, float min, float max, uint randType)
{
    // TODO: Have rand return 0 according to flags & randType instead of
    // running it multiple times when not needed?
    return (randNum * (max - min) * isFlagContained(randType, flags)) + min;
}

float2 randRangeVec2(float randNum, float2 min, float2 max, uint randType)
{
    return lerp(min, max, randNum * isFlagContained(randType, flags));
}

float3 randRangeVec3(float randNum, float3 min, float3 max, uint randType)
{
    return lerp(min, max, randNum * isFlagContained(randType, flags));
}

float4 randRangeVec4(float randNum, float4 min, float4 max, uint randType)
{
    return lerp(min, max, randNum * isFlagContained(randType, flags));
}

float3 randVec3(float3 randNum, float3 minComps, float3 maxComps, uint randType)
{
    return float3(randRangeFloat(randNum.x, minComps.x, maxComps.x, randType),
        randRangeFloat(randNum.y, minComps.y, maxComps.y, randType),
        randRangeFloat(randNum.z, minComps.z, maxComps.z, randType));

}

// CBUFFER b0 DECLARED IN EMITTERLAYOUT.H

cbuffer numDeadParticles : register(b1)
{
    uint numDeadParticles; // number of dead particles, prevent overconsumption
    uint3 padding; // padding for 16byte alignment 
};

// Actual particle information
// See header for more information about the structs
RWStructuredBuffer<Particle> particlePool		: register(u0);	// All particles
ConsumeStructuredBuffer<ParticleDead> deadList	: register(u1); // Dead particles coming from AppendStructuredBuffer

// Ran for number of particles in the dead list
[numthreads(NUM_PARTICLE_THREADS, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // Ensure we dont overconsume!
    if(DTid.x < numDeadParticles && DTid.x < numToEmit)
    {
        // Gen first rand num
        float4 randNums = hash41(DTid.x + nonce);

        // Generate random age
        float randAge = randRangeFloat(randNums.x, minAge, maxAge, RAND_AGE);

        // Create new particle to copy
        Particle particle;

        // Non random stuff
        particle.sAlpha = iAlpha;
        particle.eAlpha = eAlpha;
        
        // Back to random stuff
        particle.currAge = randAge;
        particle.maxAge = randAge;
        particle.direction = normalize(randVec3(randNums.yzw, minXYZTheta, maxXYZTheta, RAND_DIR)); // unsure if this is correct...., unsure if normalize required

        randNums = hash41(randNums.y + nonce);
        particle.sTint = randRangeVec3(randNums.x, iMinTint, iMaxTint, RAND_ITINT);
        particle.eTint = randRangeVec3(randNums.y, eMinTint, eMaxTint, RAND_ETINT);
        particle.sSize = randRangeVec2(randNums.z, iMinSize, iMaxSize, RAND_ISIZE);
        particle.eSize = randRangeVec2(randNums.w, eMinSize, eMaxSize, RAND_ESIZE);

        randNums = hash41(randNums.z + nonce);
        particle.sSpeed = randRangeFloat(randNums.x, iMinSpeed, iMaxSpeed, RAND_ISPEED);
        particle.eSpeed = randRangeFloat(randNums.y, eMinSpeed, eMaxSpeed, RAND_ESPEED);
        particle.worldPos = position;
        particle.flags = flags;
        particle._padding = 0;

        // Grab index of dead particle
        uint pIndex = deadList.Consume().index;

	    // Copy it over!
        particlePool[pIndex] = particle;
    }
}