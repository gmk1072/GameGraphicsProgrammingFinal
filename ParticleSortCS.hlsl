// SHAMEFULLY COPIED FROM https://github.com/GPUOpen-LibrariesAndSDKs/GPUParticles11/blob/master/gpuparticles11/src/Shaders/ParticleSort.hlsl

//--------------------------------------------------------------------------------------
// File: ParticleSort.hlsl
//
// The HLSL file for sorting particles
// 
// Author: Gareth Thomas
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "ShaderConstants.h"
#include "ParticleLayout.h"

RWStructuredBuffer<ParticleAlive> aliveList : register(u0);


cbuffer SortConstants : register(b0)
{
    uint4 g_kj;
}

cbuffer ActiveListCount : register(b1)
{
    int g_NumActiveParticles;
    int3 pad;
};


[numthreads(NUM_PARTICLE_THREADS, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index_low = DTid.x & (g_kj.y - 1);
    uint index_high = 2 * (DTid.x - index_low);
    uint index = index_high + index_low;
    uint nSwapElem = index ^ g_kj.y;

    // Grabbing random indicies but sorting the distances
    ParticleAlive a = aliveList[index];
    ParticleAlive b = aliveList[nSwapElem];

    // if this is true, could either be > comp or alternating
    bool whichSide = ((index & g_kj.w) == 0);
    if(!whichSide)
    {
        if(a.distanceSq > b.distanceSq)
        {
            aliveList[index] = b;
            aliveList[nSwapElem] = a;
        }
    }
    else
    {
        if(a.distanceSq < b.distanceSq)
        {
            aliveList[index] = b;
            aliveList[nSwapElem] = a;
        }
    }
}

// SubSize == 2
// CompDist == 1
// 7 3 6 8 1 4 2 5
// 0 1 2 3 4 5 6 7

// 0 > 2, no   
// 0 & 2 == 0, SHOULD BE T
// 7 > 3, T
// SWAP

// 2 > 2, no
// 2 & 2 == 0, SHOULD BE F // 10 & 10 == 2
// 6 > 8, F
// SWAP

// 4 > 2, yes
// 4 & 2 == 0, SHOULD BE T // 100 & 10 == 0 
// 1 > 4, F
// NOSWAP

// 6 > 2, yes
// 6 & 2 == 0, SHOULD BE F
// 2 > 5, F
// SWAP

