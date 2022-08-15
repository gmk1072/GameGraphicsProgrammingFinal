#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <assert.h>
#include <unordered_map>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <algorithm>
#include "UIPanel.h"
#include "Material.h"
#include "Camera.h"
#include "Mesh.h"
#include "Lights.h"
#include "ShaderConstants.h"
#include "Entity.h"
#include "DXWindow.h"

// Renderers
#include "ParticleRenderer.h"
#include "SkyRenderer.h"
#include "LightRenderer.h"

#include "GameState.h"

// defines
#define BUFFER_COUNT 4
#define BLUR_DISTANCE 4
//#define MAX_BLUR_DISTANCE 12

// We can include the correct library files here
// instead of in Visual Studio settings if we want
#pragma comment(lib, "d3d11.lib")

class Entity; // forward declaration to fix cyclic dep

class Renderer
{
	friend class ParticleRenderer;
	friend class LightRenderer;
	friend class SkyRenderer;
public:
	// Instance specific stuff
	static Renderer * const Initialize(DXWindow* const window);
	static Renderer * const Instance();
	static void Shutdown();

	// Renderer functions
	void StageEntity(Entity* const entity);
	void UnstageEntity(Entity * const entity);
	void Render(const Camera * const camera);
	void UpdateCS(float dt, float totalTime); // update exclusively for compute shader use
	void OnResize(unsigned int width, unsigned int height);

	// UI Renderer Specific
	// Sets the current panel to draw
	void SetCurrentPanel(UIPanel * panel);

	// Load a font from a given path onto the GPU
	// TODO: CONVERT TO CREATE FONT AND HANDLE FONTS SOMEWHERE ELSE?
	void LoadFont(const char* const name, const wchar_t * const path);

	// Shader factory. CALLER SHOULD FREE CREATED VARIABLES
	SimpleVertexShader* const CreateSimpleVertexShader() const;
	SimplePixelShader* const CreateSimplePixelShader() const;
	SimpleComputeShader * const CreateSimpleComputeShader() const;
	ParticleEmitter* const CreateContinuousParticleEmitter(std::string name, unsigned int particlesPerSeconds, float seconds) const;
	ParticleEmitter* const CreateBurstParticleEmitter(std::string name, unsigned int numParticles) const;

	// Mesh factory. CALLER SHOULD FREE CREATED VARIABLES
	Mesh* const CreateMesh(const char* path) const;

	// Texture factory. CALLER SHOULD FREE CREATED VARIABLES
	Texture2D* const CreateTexture2D(const wchar_t * path, Texture2DType type, Texture2DFileType fileType = Texture2DFileType::OTHER);

	// Get the map of material IDs to entity pointers that are to be rendered
	const std::unordered_multimap<unsigned int, Entity*>& GetRenderBatches() const;

	// Removes all emitters from particle renderer
	void ReleaseParticleRenderer();

private:
	// Instance specific stuff
	Renderer(DXWindow* const window);
	~Renderer();
	static Renderer* instance;

	// Initializing DXCORE
	HRESULT InitDirectX(DXWindow* const window);

	// Deferred rendering
	inline void ClearRenderTargets();

	// Rendering UI
	inline void RenderUI();

	// -- DXCORE --
	D3D_FEATURE_LEVEL		dxFeatureLevel;
	IDXGISwapChain*			swapChain;
	ID3D11Device*			device;
	ID3D11DeviceContext*	context;
	ID3D11RenderTargetView* backBufferRTV;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11DepthStencilState* lightStencilState;

	// -- DEFERRED RENDERING --
	ID3D11Texture2D* depthBufferTexture;
	ID3D11Texture2D* targetTexts[BUFFER_COUNT];
	ID3D11RenderTargetView* targetViews[BUFFER_COUNT];
	ID3D11ShaderResourceView* targetSRVs[BUFFER_COUNT];
	ID3D11SamplerState* targetSampler;
	ID3D11BlendState* addBlendState;
	SimpleVertexShader* deferredVS;
	

	ID3D11Texture2D* postProcessTexts[3];
	ID3D11RenderTargetView* postProcessRTVs[3];//with lighting, bloom pixels, glow pixels
	ID3D11ShaderResourceView* postProcessSRVs[3];

	ID3D11Texture2D* halfTexts[2];//half width and half height textures
	ID3D11RenderTargetView* halfRTVs[2];
	ID3D11ShaderResourceView* halfSRVs[2];
	ID3D11ShaderResourceView* depthSRV;

	D3D11_VIEWPORT viewport;
	D3D11_VIEWPORT halfViewport;

	// -- POSTPROCESSING GLOW --
	float texelWidth;	//change on resize
	float texelHeight;	//change on resize
	float blurDist;	//not const or define so it can change later (ie. settings if we ever have them)
	float colorThreshold;
	float glowPercentage;
	float glowDist;

	SimplePixelShader* volumetricLightingPS;
	SimplePixelShader* downsamplePS;
	SimplePixelShader* upsamplePS;
	SimplePixelShader* horizontalBlurPS;
	SimplePixelShader* verticalBlurPS;
	SimplePixelShader* postPS;

	// -- UI --
	UIPanel* panel;

	// Pointer to single sprite batch which will be used to render all UI
	SpriteBatch* spriteBatch;

	// -- TEXTURE --
	ID3D11SamplerState* objectTextureSampler;

	// -- PARTICLES --
	ParticleRenderer* particleRenderer;

	// -- SKY --
	SkyRenderer* skyRenderer;

	// -- LIGHTING --
	SimplePixelShader* prePostProcessPS;
	LightRenderer* lightRenderer;

	// Lights
	SimpleVertexShader* deferredLightVS;
	SimplePixelShader* deferredLightingPS;

	// -- RENDER BATCHES --
	// For every material, render all the meshes!
	// Cool C++11 data structure that allows multiple items to the same key.
	// Exactly what I needed!
	std::unordered_multimap<unsigned int, Entity*> renderBatches;

	// Map by font name
	std::unordered_map<const char*, SpriteFont*> fontMap;

};

