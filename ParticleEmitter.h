#pragma once

// DEBUG
#include <stdio.h>

#include <memory.h>
#include "EmitterLayout.h"
#include "ParticleLayout.h"
#include "ShaderConstants.h"

enum class ParticleEmitterType
{
	BURST,		// numParticles emitted ONCE, wait for emitter lifetime to die down
	CONTINOUS	// numParticles, but particles emitted every some number of frames
};

class ParticleEmitter
{
	// Allow particle renderer to directly reference emitter information
	friend class ParticleRenderer;
public:
	// Non-random
	void SetTint(DirectX::XMFLOAT3& initialTint);
	void SetEndTint(DirectX::XMFLOAT3& endTint);
	void SetSpeed(float initialSpeed);
	void SetEndSpeed(float endSpeed);
	void SetSize(DirectX::XMFLOAT2& initialSize);
	void SetEndSize(DirectX::XMFLOAT2& endSize);
	void SetAlpha(float initialAlpha);
	void SetEndAlpha(float endAlpha);
	void SetDirection(DirectX::XMFLOAT3& dir);
	void SetAge(float age);
	void SetPosition(const DirectX::XMFLOAT3& pos);
	void SetNonce(unsigned int nonce);
	bool SetTextureID(unsigned int textureID);

	// Random ranges
	// If any of these are set, the initial values set using the correlating functions above
	// are overwritten and ignored.
	void SetInitialTintRange(DirectX::XMFLOAT3& initialMinTint, DirectX::XMFLOAT3& initialMaxTint);
	void SetEndTintRange(DirectX::XMFLOAT3& endMinTint, DirectX::XMFLOAT3& endMaxTint);
	void SetInitialSpeedRange(float initialMinSpeed, float initialMaxSpeed);
	void SetEndSpeedRange(float endMinSpeed, float endMaxSpeed);
	void SetInitialSizeRange(DirectX::XMFLOAT2& initialMinSize, DirectX::XMFLOAT2& initialMaxSize);
	void SetEndSizeRange(DirectX::XMFLOAT2& endMinSize, DirectX::XMFLOAT2& endMaxSize);
	void SetDirectionRange(DirectX::XMFLOAT3& minDir, DirectX::XMFLOAT3& maxDir);
	void SetAgeRange(float minAge, float maxAge);

	// Interp options
	void SetInterpAlpha(bool val);
	void SetInterpSpeed(bool val);
	void SetInterpSize(bool val);
	void SetInterpTint(bool val);
	
	// Loop options
	void SetLoop(int loopAmount);

	// Mark as ready to emit
	void Emit();
private:
	ParticleEmitter(unsigned int numParticles);
	ParticleEmitter(unsigned int particlesPerRate, float rate);
	~ParticleEmitter();

	bool CanEmit(float dt);

	Emitter emitter;
	ParticleEmitterType type;
	float counter; // as long as the emitters largest age, max(minAge, maxAge), no overconsumption
	float emitRate;
	unsigned int numParticles;
	unsigned int numParticlesAligned;
	int numLoops;
	int currNumLoops;
	bool isActive;
	bool isLoopable;
};

