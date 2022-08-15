#ifndef PARTICLE_LAYOUT_H
#define PARTICLE_LAYOUT_H

#include "ShaderTypes.h"

// Particle emitter flags
#define RAND_AGE		0x1
#define INTERP_ALPHA	0x2 // interpolate alpha value
#define RAND_DIR		0x4
#define	RAND_ISPEED		0x8
#define RAND_ESPEED		0x10
#define RAND_ISIZE		0x20
#define RAND_ESIZE		0x40
#define RAND_ITINT		0x80
#define RAND_ETINT		0x100
#define INTERP_SPEED	0x200
#define INTERP_SIZE		0x400
#define INTERP_TINT		0x800 // 12 bits
#define TEXT_ID_SIZE	20
// PLEASE NOTE, LAST 20 BITS PRESERVED FOR TEXTURE INDEX

struct Particle
{
	// Starting particle information
	float3 sTint;		// Start tint
	float sAlpha;		// Start alpha
	float3 worldPos;	// World position
	float currAge;		// Current age
	float3 direction;	// Current direction
	float sSpeed;		// Start speed
	float2 sSize;		// Start size

	// Ending particle information
	float2 eSize;		// Ending size
	float3 eTint;		// End tint
	float eAlpha;		// End alpha
	float eSpeed;		// Ending speed
	uint flags;			// flags of this particle taken from emitter
	float maxAge;		// Max age of this particle, used for interp
	float _padding;	// Padding for 16 byte alignment
};

struct ParticleAlive
{
	uint index;			// Index of alive particle in particlePool
	float distanceSq;	// Distance squared of particlePool[index] from camera
	float2 padding;		// padding for 16 byte alignment
};

struct ParticleDead
{
	uint index;			// Index of dead particle in particlePool
	float3 padding;		// padding for 16 byte alignment
};

struct ParticleVertexToPixel
{
	float4 position		SEMANTIC(SV_Position);
	float4 tint			SEMANTIC(COLOR);
	float3 worldPos		SEMANTIC(POSITION);
	float age			SEMANTIC(AGE);
	//float3 velocity		SEMANTIC(VELOCITY);
	//float3 normal		SEMANTIC(NORMAL);
	//float3 tangent		SEMANTIC(TANGENT);
	float2 uv			SEMANTIC(TEXCOORD);
};


#endif