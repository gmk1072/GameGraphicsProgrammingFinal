#ifndef EMITTER_LAYOUT
#define EMITTER_LAYOUT

#include "ShaderTypes.h"

// Struct/cbuffer which holds particle emitter information.
CBUFFER(Emitter, b0)
{
	// Fade out
	// Speed
	// Color
	float3 iMinTint;
	float iAlpha;
	float3 iMaxTint;
	float eAlpha;
	float3 eMinTint;
	float iMinSpeed;
	float3 eMaxTint;
	float iMaxSpeed;

	// Size
	float2 iMinSize;
	float2 iMaxSize;
	float2 eMinSize;
	float2 eMaxSize;
	
	// Age
 	// Direction
	float3 minXYZTheta;
	float minAge;
	float3 maxXYZTheta;
	float maxAge;
	// // or just spherical, or just a line

	// Emitter flags and position
	float3 position; // World position of emitter
	uint flags;

	// Speed
	float eMinSpeed;
	float eMaxSpeed;

	// CPU generated random number (nonce) used for hashing function
	uint nonce;
	uint numToEmit;
};

#endif // !EMITTER_LAYOUT
