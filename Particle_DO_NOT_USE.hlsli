// ALL BASED ON http://twvideo01.ubm-us.net/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf
// WITH HELP FROM CHRIS Cascioli

#ifndef PARTICLE_H
#define PARTICLE_H

struct Particle
{
	float4 color;		// Color of particle (solid colors for now)
	float3 worldPos;	// Current WORLD position
	float age;			// Age in seconds (?)
	float3 velocity;	// Current velocity
	float padding;
};

struct ParticleVertexToPixel
{
    float4 position : SV_Position;
    float4 tint : COLOR;
    float3 worldPos : POSITION;
    float age : AGE;
    float3 velocity : VELOCITY;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

// TODO: CONVERT TO FLOAT2 RW AND CAST
struct ParticleAlive
{
	uint index;			// Index of alive particle in particlePool
	float distanceSq;	// Distance squared of particlePool[index] from camera
	float2 padding;
};

// TODO: CONVERT TO UINT RW
struct ParticleDead
{
	uint index;			// Index of dead particle in particlePool
	float3 padding;
};

#endif