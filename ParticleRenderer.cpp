#include "ParticleRenderer.h"
#include "MemoryDebug.h"

// Null pointer arrays used to unbind UAVs and SRVs
static ID3D11UnorderedAccessView* nullUAVs[3] = { nullptr, nullptr, nullptr };
static ID3D11ShaderResourceView* nullSRVs[3] = { nullptr, nullptr, nullptr };

// --------------------------------------------------------
// Constructor
//
// Creates a particle renderer which will use the context and
// device found in the given renderer.
//
// renderer - Reference to renderer object in which to render
//			  particles to.
// --------------------------------------------------------
ParticleRenderer::ParticleRenderer(Renderer & renderer) :
	renderer(renderer)
{
	// Init all to null
	particlePool = nullptr;
	aliveList = nullptr;
	deadList = nullptr;
	drawArgs = nullptr;
	numDeadParticlesCBuffer = nullptr;
	numAliveParticlesCBuffer = nullptr;
	particleIndexBuffer = nullptr;
	particlePoolSRV = nullptr;
	aliveListSRV = nullptr;
	deadListSRV = nullptr;
	particlePoolUAV = nullptr;
	aliveListUAV = nullptr;
	deadListUAV = nullptr;
	drawArgsUAV = nullptr;
	particleEmitCS = nullptr;
	particleUpdateCS = nullptr;
	particleDrawArgsCS = nullptr;
	particleInitCS = nullptr;
	particleVS = nullptr;
	particleDeferredPS = nullptr;
	maxParticles = 131072;

	// Init random
	srand(static_cast<unsigned int>(time(nullptr)));
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
ParticleRenderer::~ParticleRenderer()
{
}

// --------------------------------------------------------
// Initializes the particle renderer.
//
// returns - Result of initialization.
// --------------------------------------------------------
HRESULT ParticleRenderer::Initialize()
{
	// Load in shaders
	particleEmitCS = renderer.CreateSimpleComputeShader();
	if (!particleEmitCS->LoadShaderFile(L"./Assets/Shaders/ParticleEmitCS.cso"))
		return E_FAIL;
	particleUpdateCS = renderer.CreateSimpleComputeShader();
	if (!particleUpdateCS->LoadShaderFile(L"./Assets/Shaders/ParticleUpdateCS.cso"))
		return E_FAIL;
	particleDrawArgsCS = renderer.CreateSimpleComputeShader();
	if (!particleDrawArgsCS->LoadShaderFile(L"./Assets/Shaders/ParticleDrawArgsCS.cso"))
		return E_FAIL;
	particleInitCS = renderer.CreateSimpleComputeShader();
	if (!particleInitCS->LoadShaderFile(L"./Assets/Shaders/ParticleInitCS.cso"))
		return E_FAIL;
	particleSortCS = renderer.CreateSimpleComputeShader();
	if (!particleSortCS->LoadShaderFile(L"./Assets/Shaders/ParticleSortCS.cso"))
		return E_FAIL;
	particleDeferredPS = renderer.CreateSimplePixelShader();
	if (!particleDeferredPS->LoadShaderFile(L"./Assets/Shaders/ParticleDefferedPS.cso"))
		return E_FAIL;
	particleForwardPS = renderer.CreateSimplePixelShader();
	if (!particleForwardPS->LoadShaderFile(L"./Assets/Shaders/particleForwardPS.cso"))
		return E_FAIL;
	particleVS = renderer.CreateSimpleVertexShader();
	if (!particleVS->LoadShaderFile(L"./Assets/Shaders/ParticleVS.cso"))
		return E_FAIL;

	// Error reporting
	HRESULT hr = S_OK;

	// Create buffers (I know I could have reused structs but I wanted to be as
	//				   verbose as possible since this is the first time im doing
	//				   this)
	// Particle pool buffer is only a RWStructured w/o incrementer
	D3D11_BUFFER_DESC particlePoolDesc = {};
	particlePoolDesc.Usage = D3D11_USAGE_DEFAULT; // Defaults = GPU R/W only
	particlePoolDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE; // UAV and Shader Resource
	particlePoolDesc.ByteWidth = sizeof(Particle) * maxParticles; // number of particles in particle pool
	particlePoolDesc.CPUAccessFlags = 0;
	particlePoolDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; // needs to be specified when structured buffer
	particlePoolDesc.StructureByteStride = sizeof(Particle); // needs to be specified when structured buffer
	hr = renderer.device->CreateBuffer(&particlePoolDesc, nullptr, &particlePool);
	if (FAILED(hr))
		return hr;

	// Alive list uses incrementer and will be bound to compute and shader
	D3D11_BUFFER_DESC aliveListDesc = {};
	aliveListDesc.Usage = D3D11_USAGE_DEFAULT; // Defaults = GPU R/W only
	aliveListDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS; // UAV and Shader Resource
	aliveListDesc.ByteWidth = sizeof(ParticleAlive) * maxParticles;
	aliveListDesc.CPUAccessFlags = 0;
	aliveListDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; // needs to be specified when structured buffer
	aliveListDesc.StructureByteStride = sizeof(ParticleAlive); // needs to be specified when structured buffer
	hr = renderer.device->CreateBuffer(&aliveListDesc, nullptr, &aliveList);
	if (FAILED(hr))
		return hr;

	// Dead list is append/consume but is also a structured buffer.
	// Will NOT be a shader resource since we dont need to bidn this to any 
	// shader other than compute.
	D3D11_BUFFER_DESC deadListDesc = {};
	deadListDesc.Usage = D3D11_USAGE_DEFAULT; // Defaults = GPU R/W only
	deadListDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS; // UAV and Shader Resource
	deadListDesc.ByteWidth = sizeof(ParticleDead) * maxParticles;
	deadListDesc.CPUAccessFlags = 0;
	deadListDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; // needs to be specified when structured buffer
	deadListDesc.StructureByteStride = sizeof(ParticleDead); // needs to be specified when structured buffer
	hr = renderer.device->CreateBuffer(&deadListDesc, nullptr, &deadList);
	if (FAILED(hr))
		return hr;

	// Buffer for where draw args are placed
	D3D11_BUFFER_DESC argDesc = {};
	argDesc.Usage = D3D11_USAGE_DEFAULT; // Defaults = GPU R/W only
	argDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS; // UAV and Shader Resource
	argDesc.ByteWidth = sizeof(unsigned int) * 5;
	argDesc.CPUAccessFlags = 0;
	argDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS; // special flag to be used for args
	argDesc.StructureByteStride = sizeof(unsigned int);
	hr = renderer.device->CreateBuffer(&argDesc, nullptr, &drawArgs);
	if (FAILED(hr))
		return hr;

	// Will keep track of how many particles are dead.
	D3D11_BUFFER_DESC partNumBuffDesc = {};
	partNumBuffDesc.Usage = D3D11_USAGE_DEFAULT; // Defaults = GPU R/W only
	partNumBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	partNumBuffDesc.ByteWidth = 4 * sizeof(unsigned int);
	partNumBuffDesc.CPUAccessFlags = 0;
	partNumBuffDesc.MiscFlags = 0;
	partNumBuffDesc.StructureByteStride = 0;
	hr = renderer.device->CreateBuffer(&partNumBuffDesc, nullptr, &numDeadParticlesCBuffer);
	if (FAILED(hr))
		return hr;
	hr = renderer.device->CreateBuffer(&partNumBuffDesc, nullptr, &numAliveParticlesCBuffer);
	if (FAILED(hr))
		return hr;

	// Will keep track of index buffer related information
	D3D11_BUFFER_DESC indexBuffDesc = {};
	indexBuffDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBuffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffDesc.ByteWidth = sizeof(unsigned int) * 6 * maxParticles;
	indexBuffDesc.CPUAccessFlags = 0;
	indexBuffDesc.MiscFlags = 0;
	indexBuffDesc.StructureByteStride = 0;

	// Generate indices
	unsigned int* indices = new unsigned int[maxParticles * 6];
	for (unsigned int i = 0, baseCounter = 0;
		i < maxParticles;
		i+=6, baseCounter+=4)
	{
		indices[i + 0] = baseCounter + 0;
		indices[i + 1] = baseCounter + 1;
		indices[i + 2] = baseCounter + 2;
		indices[i + 3] = baseCounter + 2;
		indices[i + 4] = baseCounter + 1;
		indices[i + 5] = baseCounter + 3;
	}

	// Prepare to upload
	D3D11_SUBRESOURCE_DATA indexBuffData = {};
	indexBuffData.pSysMem = indices;
	indexBuffData.SysMemPitch = 0;
	indexBuffData.SysMemSlicePitch = 0;
	hr = renderer.device->CreateBuffer(&indexBuffDesc, &indexBuffData, &particleIndexBuffer);
	delete[] indices;
	if (FAILED(hr))
		return hr;

	// Create UAVs
	// Particle pool UAV, nothing special here
	D3D11_UNORDERED_ACCESS_VIEW_DESC particlePoolUAVDesc = {};
	particlePoolUAVDesc.Format = DXGI_FORMAT_UNKNOWN; // must be used for structured/append/consume buffers
	particlePoolUAVDesc.Buffer.FirstElement = 0;
	particlePoolUAVDesc.Buffer.Flags = 0;
	particlePoolUAVDesc.Buffer.NumElements = maxParticles;
	particlePoolUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // Unsure about this...
	hr = renderer.device->CreateUnorderedAccessView(particlePool, &particlePoolUAVDesc, &particlePoolUAV);
	if (FAILED(hr))
		return hr;

	// Alive list UAV, special flag will be a counter!
	D3D11_UNORDERED_ACCESS_VIEW_DESC aliveListUAVDesc = {};
	aliveListUAVDesc.Format = DXGI_FORMAT_UNKNOWN; // must be used for structured/append/consume buffers
	aliveListUAVDesc.Buffer.FirstElement = 0;
	aliveListUAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER; // SPECIAL FLAG FOR USING INCREMENT/DECREMENT FUNCTIONS
	aliveListUAVDesc.Buffer.NumElements = maxParticles;
	aliveListUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // Unsure about this...
	hr = renderer.device->CreateUnorderedAccessView(aliveList, &aliveListUAVDesc, &aliveListUAV);
	if (FAILED(hr))
		return hr;

	// Dead list UAV, special flag will be append/consume
	D3D11_UNORDERED_ACCESS_VIEW_DESC deadListUAVDesc = {};
	deadListUAVDesc.Format = DXGI_FORMAT_UNKNOWN; // must be used for structured/append/consume buffers
	deadListUAVDesc.Buffer.FirstElement = 0;
	deadListUAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; // SPECIAL FLAG FOR USING APPEND/CONSUME BUFFERS
	deadListUAVDesc.Buffer.NumElements = maxParticles;
	deadListUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // Unsure about this...
	hr = renderer.device->CreateUnorderedAccessView(deadList, &deadListUAVDesc, &deadListUAV);
	if (FAILED(hr))
		return hr;

	// Dead list UAV, special flag will be append/consume
	D3D11_UNORDERED_ACCESS_VIEW_DESC argUAVDesc = {};
	argUAVDesc.Format = DXGI_FORMAT_R32_TYPELESS; // must be used for structured/append/consume buffers
	argUAVDesc.Buffer.FirstElement = 0;
	argUAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW; // Required for raw view buffers
	argUAVDesc.Buffer.NumElements = 5; // 5 arguments for indirect indexed draw
	argUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // Unsure about this...
	hr = renderer.device->CreateUnorderedAccessView(drawArgs, &argUAVDesc, &drawArgsUAV);
	if (FAILED(hr))
		return hr;

	// Create SRVs (SRVs are the same for both particlePool and aliveList)
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = 0;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	hr = renderer.device->CreateShaderResourceView(particlePool, &srvDesc, &particlePoolSRV);
	if (FAILED(hr))
		return hr;
	hr = renderer.device->CreateShaderResourceView(aliveList, &srvDesc, &aliveListSRV);
	if (FAILED(hr))
		return hr;

	// Load in the particle texture ATLAS
	particleTextureAtlas = renderer.CreateTexture2D(L"./Assets/Textures/atlas.dds", Texture2DType::ALBEDO, Texture2DFileType::DDS);

	// Initialize dead list
	InitialEmitParticles();

	return S_OK;
}

// --------------------------------------------------------
// Cleans up and shutsdown this particle renderer object.
//
// returns - Result of shutdown.
// --------------------------------------------------------
HRESULT ParticleRenderer::Shutdown()
{
	// Release buffers
	if (particlePool) { particlePool->Release(); }
	if (aliveList) { aliveList->Release(); }
	if (deadList) { deadList->Release(); }
	if (drawArgs) { drawArgs->Release(); }
	if (numDeadParticlesCBuffer) { numDeadParticlesCBuffer->Release(); }
	if (numAliveParticlesCBuffer) { numAliveParticlesCBuffer->Release(); }
	if (particleIndexBuffer) { particleIndexBuffer->Release(); }
	if (particlePoolSRV) { particlePoolSRV->Release(); }
	if (aliveListSRV) { aliveListSRV->Release(); }
	if (deadListSRV) { deadListSRV->Release(); }
	if (particlePoolUAV) { particlePoolUAV->Release(); }
	if (aliveListUAV) { aliveListUAV->Release(); }
	if (deadListUAV) { deadListUAV->Release(); }
	if (drawArgsUAV) { drawArgsUAV->Release(); }


	// Release shaders
	if (particleEmitCS) { delete particleEmitCS; }
	if (particleUpdateCS) { delete particleUpdateCS; }
	if (particleDrawArgsCS) { delete particleDrawArgsCS; }
	if (particleInitCS) { delete particleInitCS; }
	if (particleSortCS) { delete particleSortCS; }
	if (particleDeferredPS) { delete particleDeferredPS; }
	if (particleForwardPS) { delete particleForwardPS; }
	if (particleVS) { delete particleVS; }

	// Release textures
	if (particleTextureAtlas) { delete particleTextureAtlas; }

	Release();

	return S_OK;
}

// --------------------------------------------------------
// Deletes all currently staged particle emitters.
// --------------------------------------------------------
void ParticleRenderer::Release()
{
	// Free all emitters and reset map
	for (auto it = particleEmitters.begin(); it != particleEmitters.end(); it++)
		delete it->second;
	particleEmitters.clear();
}

// --------------------------------------------------------
// Creates a continuous particle emitter.
//
// NOTE: The name of the particle emitter must not already exist
//		 in the particle renderer.
//
// name - Unique name for this particle emitter.
// particlesPerSeconds - Number of particles to emit per
//						 given rate in seconds.
// seconds - Number of seconds that need to pass before 
//			 emitting particlesPerSeconds particles.
//
// return - Pointer to particle emitter object.
// --------------------------------------------------------
ParticleEmitter * const ParticleRenderer::CreateContinuousParticleEmitter(std::string name, unsigned int particlesPerSeconds, float seconds)
{
	// Ensure name doesn't already exist
	assert(particleEmitters.count(name) == 0);
	ParticleEmitter* particleEmitter = new ParticleEmitter(particlesPerSeconds, seconds);
	particleEmitters[name] = particleEmitter;
	return particleEmitter;
}

// --------------------------------------------------------
// Creates a burst particle emitter.
//
// NOTE: The name of the particle emitter must not already exist
//		 in the particle renderer.
//
// name - Unique name for this particle emitter.
// numParticles - Number of particles to emit when Emit is
//				  called on this particle emitter.
//
// return - Pointer to particle emitter object.
// --------------------------------------------------------
ParticleEmitter * const ParticleRenderer::CreateBurstParticleEmitter(std::string name, unsigned int numParticles)
{
	// Ensure name doesn't already exist
	assert(particleEmitters.count(name) == 0);
	ParticleEmitter* particleEmitter = new ParticleEmitter(numParticles);
	particleEmitters[name] = particleEmitter;
	return particleEmitter;
}

// --------------------------------------------------------
// Emits particles that need to be emitted this frame.
// Dispatches particle update to compute shader.
//
// NOTE: D3D11 state values are not preserved.
//
// dt - Delta time.
// totalTime - Total game time.
// --------------------------------------------------------
void ParticleRenderer::Update(float dt, float totalTime)
{
	// Update and process particles
	for (auto it = particleEmitters.begin(); it != particleEmitters.end(); it++)
		if (it->second->CanEmit(dt))
			EmitParticles(*it->second);
	UpdateParticles(dt);
	ProcessDrawArgs();
}

// --------------------------------------------------------
// Renders all living particles to the renderers deferred
// RTV's.
//
// NOTE: D3D11 state values are not preserved.
//
// camera - Camera to use when rendering particles.
// --------------------------------------------------------
void ParticleRenderer::Render(const Camera * const camera)
{
	RenderParticles(camera);
}

// --------------------------------------------------------
// Dispatches a simple compute shader that initializes the
// particle dead list with values 0 to maxParticles.
// --------------------------------------------------------
inline void ParticleRenderer::InitialEmitParticles()
{
	// Bind dead list since this is to be filled
	// Set initial count of dead particles to 0, this is going to be filled after dispatch
	bool result;
	result = particleInitCS->SetUnorderedAccessView("deadList", deadListUAV, 0);
	particleInitCS->CopyAllBufferData();
	particleInitCS->SetShader();
	particleInitCS->DispatchByGroups(DISPATCH_DIV(maxParticles), 1, 1);

	// Unbind
	renderer.context->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);
}

// --------------------------------------------------------
// Emit particles from a given particle emitter.
//
// NOTE: D3D11 state values are not preserved.
//
// particleEmitter - Reference to particle emitter that 
//					 contains particle information.
// --------------------------------------------------------
inline void ParticleRenderer::EmitParticles(ParticleEmitter& particleEmitter)
{
	// Set nonce (handle in emitter class)
	particleEmitter.SetNonce(rand());

	// Bind particle pool and dead list
	bool result;
	result = particleEmitCS->SetUnorderedAccessView("particlePool", particlePoolUAV, -1);
	result = particleEmitCS->SetUnorderedAccessView("deadList", deadListUAV, -1);
	result = particleEmitCS->SetStruct("iMinTint", &particleEmitter.emitter, sizeof(Emitter));
	particleEmitCS->CopyAllBufferData();

	// Copy number of dead particles to constant buffer
	renderer.context->CopyStructureCount(numDeadParticlesCBuffer, 0, deadListUAV);

	ID3D11Buffer* buffers[2] = { particleEmitCS->GetBufferInfo("Emitter")->ConstantBuffer, numDeadParticlesCBuffer };

	// Set the constant buffer in addition to the previously set cbuffer
	renderer.context->CSSetConstantBuffers(0, 2, buffers);
	renderer.context->CSSetShader(particleEmitCS->GetDirectXShader(), nullptr, 0);

	// Dispatch
	particleEmitCS->DispatchByGroups(DISPATCH_DIV(particleEmitter.numParticlesAligned), 1, 1);

	// Unbind
	renderer.context->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
}

// --------------------------------------------------------
// Dispatches the particle update compute shader.
//
// NOTE: D3D11 state values are not preserved.
//
// dt - Delta time.
// --------------------------------------------------------
inline void ParticleRenderer::UpdateParticles(float dt)
{
	bool result;
	result = particleUpdateCS->SetUnorderedAccessView("particlePool", particlePoolUAV, -1);
	result = particleUpdateCS->SetUnorderedAccessView("aliveList", aliveListUAV, 0); // alive is redone every frame
	result = particleUpdateCS->SetUnorderedAccessView("deadList", deadListUAV, 0); // dead is persistent
	result = particleUpdateCS->SetFloat3("cameraPos", DirectX::XMFLOAT3(0,0,0));
	result = particleUpdateCS->SetFloat("dt", dt);
	particleUpdateCS->CopyAllBufferData();
	renderer.context->CSSetConstantBuffers(0, 1, &particleUpdateCS->GetBufferInfo("externalData")->ConstantBuffer);
	renderer.context->CSSetShader(particleUpdateCS->GetDirectXShader(), nullptr, 0);
	particleUpdateCS->DispatchByGroups(DISPATCH_DIV(maxParticles), 1, 1); // This needs to update ONLY the particles that have been INIT'd

	// Unbind
	renderer.context->CSSetUnorderedAccessViews(0, 3, nullUAVs, nullptr);
}

// --------------------------------------------------------
// Dispatches a simple compute shader which simply copies
// the number of alive particles found on the GPU to a
// separate buffer which will act as arguments for an 
// instanced draw.
//
// NOTE: D3D11 state values are not preserved.
// --------------------------------------------------------
inline void ParticleRenderer::ProcessDrawArgs()
{
	// Copy number of alive particles to constant buffer
	renderer.context->CopyStructureCount(numAliveParticlesCBuffer, 0, aliveListUAV);

	bool result;
	result = particleDrawArgsCS->SetUnorderedAccessView("drawArgs", drawArgsUAV);
	particleDrawArgsCS->CopyAllBufferData();
	
	// Set the constant buffer to the numAliveParticles
	renderer.context->CSSetConstantBuffers(0, 1, &numAliveParticlesCBuffer);
	renderer.context->CSSetShader(particleDrawArgsCS->GetDirectXShader(), nullptr, 0);

	particleDrawArgsCS->DispatchByGroups(1, 1, 1); // run once on one thread

	// Unbind
	renderer.context->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);
}

// --------------------------------------------------------
// Renders all living particles to the renderers deferred
// RTV's.
//
// NOTE: D3D11 state values are not preserved.
//
// camera - Camera to use when rendering particles.
// --------------------------------------------------------
inline void ParticleRenderer::RenderParticles(const Camera * const camera)
{
	// CALL THIS IN RENDERER BEFORE LIGHTING CALC
	bool result;

	DirectX::XMFLOAT4X4 view = camera->GetViewMatrix();
	result = particleVS->SetMatrix4x4("view", view);

	DirectX::XMFLOAT4X4 proj = camera->GetProjectionMatrix();
	result = particleVS->SetMatrix4x4("projection", proj);
	particleVS->CopyAllBufferData();

	renderer.context->VSSetShaderResources(0, 1, &particlePoolSRV);
	renderer.context->VSSetShaderResources(1, 1, &aliveListSRV);
	renderer.context->VSSetConstantBuffers(0, 1, &particleVS->GetBufferInfo("externalData")->ConstantBuffer);
	renderer.context->VSSetShader(particleVS->GetDirectXShader(), nullptr, 0);

	// Use simple pixel shader to output stuff
	ID3D11SamplerState* sampler = particleTextureAtlas->GetSamplerState();
	ID3D11ShaderResourceView* srv = particleTextureAtlas->GetSRV();
	renderer.context->PSSetSamplers(0, 1, &sampler);
	renderer.context->PSSetShaderResources(0, 1, &srv);
	renderer.context->PSSetShader(particleDeferredPS->GetDirectXShader(), nullptr, 0);

	// Draw indirect
	renderer.context->IASetInputLayout(nullptr);
	renderer.context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	renderer.context->IASetIndexBuffer(particleIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	renderer.context->DrawIndexedInstancedIndirect(drawArgs, 0);

	// Unbind
	renderer.context->VSSetShaderResources(0, 2, nullSRVs);
}
