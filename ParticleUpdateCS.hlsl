// ALL BASED ON http://twvideo01.ubm-us.net/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf
// WITH HELP FROM CHRIS Cascioli
#include "ShaderConstants.h"
#include "ParticleLayout.h"

// Constant info for this frame
cbuffer externalData : register(b0)
{
	float3 cameraPos;	// Position of camera in world space
	float dt;			// Delta time
};

// Return 0 if the flag is not contained, otherwise, return 1
uint isFlagContained(uint theFlag, uint allFlags)
{
    return ((allFlags & theFlag) == theFlag);
}

// Actual particle information
// See header for more information about the structs
RWStructuredBuffer<Particle> particlePool		: register(u0); // All particles
RWStructuredBuffer<ParticleAlive> aliveList		: register(u1); // Alive particles (will be used for drawing)
AppendStructuredBuffer<ParticleDead> deadList	: register(u2); // Dead particles

// Ran for all particles
[numthreads(NUM_PARTICLE_THREADS, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Particle particle = particlePool[DTid.x];
    float nextAge = particle.currAge - dt;

	// Check if dead from last draw
    if (nextAge < 0.0f)
	{
		// If dead, add to deadList, will be re-emitted by the emitter shader
		ParticleDead particleDead;
		particleDead.index = DTid.x;
		particleDead.padding = float3(0, 0, 0);
		deadList.Append(particleDead);
	}
	else
	{
        // Update particle according to its flags
        particle.worldPos += (particle.direction * lerp(particle.sSpeed, particle.eSpeed, (1.0f - (particle.currAge / particle.maxAge)) * isFlagContained(INTERP_SPEED, particle.flags)) * dt);
        particle.currAge = nextAge;

		// Grab vector from camera to particle
		float3 camToParticle = particle.worldPos - cameraPos;
        float distSq = dot(camToParticle, camToParticle);

		// Add to alive list and increment counter
		ParticleAlive particleAlive;
        particleAlive.distanceSq = distSq;
		particleAlive.index = DTid.x;
		particleAlive.padding = float2(0, 0);

        uint index = aliveList.IncrementCounter();
        aliveList[index] = particleAlive;
        particlePool[DTid.x] = particle;
    }
}