#pragma once

// -- RANDOM NUMBERS, MAYBE IN OWN HEADER?? -- 
#include <cstdlib>
#include <time.h>

#include <unordered_map>

#include "Texture2D.h"
#include "ShaderConstants.h"
#include "ParticleLayout.h"
#include "EmitterLayout.h"
#include "ParticleEmitter.h"
#include "Renderer.h"

// Figures out how many groups to dispatch. To dispatch enough threads for each
// particle, we must ensure numThreadsPerGroup * groups = maxParticles.
// This macro simply solves for groups given number of particles.
#define DISPATCH_DIV(p) ((p) / NUM_PARTICLE_THREADS)

class Renderer;

// This is esentially just an extension of the renderer class
class ParticleRenderer
{
	friend class Renderer;
public:
private:
	ParticleRenderer(Renderer& renderer);
	~ParticleRenderer();

	// Initialization & Shutdown
	HRESULT Initialize();
	HRESULT Shutdown();

	// Release ALL emitters
	void Release();

	// Emitter creation
	ParticleEmitter* const CreateContinuousParticleEmitter(std::string name, unsigned int particlesPerSeconds, float seconds);
	ParticleEmitter* const CreateBurstParticleEmitter(std::string name, unsigned int numParticles);

	// Particle updating and rendering
	void Update(float dt, float totalTime);
	void Render(const Camera * const camera);

	// Pipeline update and render pipeline
	inline void InitialEmitParticles();
	inline void EmitParticles(ParticleEmitter& particleEmitter);
	inline void UpdateParticles(float dt);
	inline void ProcessDrawArgs();
	inline void RenderParticles(const Camera * const camera);

	// Particle emitter map
	std::unordered_map<std::string, ParticleEmitter*> particleEmitters;

	// Reference to renderer which will be used to setup buffers
	Renderer& renderer;
	
	// All relevant buffers
	ID3D11Buffer* particlePool;
	ID3D11Buffer* aliveList;
	ID3D11Buffer* deadList;
	ID3D11Buffer* drawArgs;
	ID3D11Buffer* numDeadParticlesCBuffer;
	ID3D11Buffer* numAliveParticlesCBuffer;
	ID3D11Buffer* particleIndexBuffer;
	ID3D11ShaderResourceView* particlePoolSRV;
	ID3D11ShaderResourceView* aliveListSRV;
	ID3D11ShaderResourceView* deadListSRV;
	ID3D11UnorderedAccessView* particlePoolUAV;
	ID3D11UnorderedAccessView* aliveListUAV;
	ID3D11UnorderedAccessView* deadListUAV;
	ID3D11UnorderedAccessView* drawArgsUAV;

	// Compute Shaders
	SimpleComputeShader* particleEmitCS;
	SimpleComputeShader* particleUpdateCS;
	SimpleComputeShader* particleDrawArgsCS;
	SimpleComputeShader* particleInitCS;
	SimpleComputeShader* particleSortCS;

	// Vertex Shader
	SimpleVertexShader* particleVS;

	// Pixel shader
	SimplePixelShader* particleDeferredPS;
	SimplePixelShader* particleForwardPS;

	// Particle Texture Atlas
	Texture2D* particleTextureAtlas;

	// Temp number of particles
	Camera* lastCamera;
	unsigned int maxParticles; // total max number of particles that can be spawned in game
};

