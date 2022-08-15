// ALL BASED ON http://twvideo01.ubm-us.net/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf
// WITH HELP FROM CHRIS Cascioli
// RESET COUNTERS EACH DRAW HERE https://www.gamedev.net/topic/662293-reset-hidden-counter-of-structuredappendconsume-buffers/
// CLEAR ALIVE LIST EACH DRAW
#include "ShaderConstants.h"
#include "ParticleLayout.h"

// Actual particle information
// See header for more information about the structs
AppendStructuredBuffer<ParticleDead> deadList : register(u0); // Dead particles coming from AppendStructuredBuffer

// Ran for all particles
[numthreads(NUM_PARTICLE_THREADS, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // Append all IDs to dead list so we can emit on first update
    ParticleDead particleDead;
    particleDead.index = DTid.x;
    particleDead.padding = float3(0, 0, 0);
    deadList.Append(particleDead);
}