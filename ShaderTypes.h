// Basic interface between CPP and HLSL structs

#ifndef SHADERTYPES_H
#define SHADERTYPES_H

// If compiling in CPP
#ifdef __cplusplus
#include <DirectXMath.h>
#define CBUFFER(name, reg) struct name
#define SEMANTIC(sem)

// Encapsulate in namespace for safety
// Same names as HLSL shaders
typedef DirectX::XMFLOAT4 float4;
typedef DirectX::XMFLOAT3 float3;
typedef DirectX::XMFLOAT2 float2;
typedef unsigned int uint;

// If compiling in HLSL
#else
#define SEMANTIC(sem) : sem
#define CBUFFER(name, reg) cbuffer name : register(reg)
#endif
#endif // !SHADERTYPES_H
