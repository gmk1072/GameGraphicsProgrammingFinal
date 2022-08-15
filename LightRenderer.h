#pragma once

#include "Renderer.h"
#include "PointLight.h"
#include "DirectionalLight.h"

class LightRenderer
{
	friend class Renderer;
public:
private:
	LightRenderer(Renderer& renderer);
	~LightRenderer();

	// Initialization & Shutdown
	HRESULT Initialize(SimpleVertexShader* quadVS, unsigned int width, unsigned int height);
	HRESULT Shutdown();

	// Light creation
	PointLight * const CreatePointLight(PointLightAttenuation attenuation, std::string name, bool autostage = false);
	DirectionalLight* const CreateDirectionalLight(std::string name, bool autostage = false);

	// Rendering related
	void Render(const Camera* const camera);
	void ClearRTV();

	// Reference to renderer to be used to setup shaders 
	Renderer& renderer;

	// Vector of all lights
	std::unordered_map<std::string, Light*> lights;

	// Vectors of staged lights
	std::vector<PointLight*> pointLights;
	std::vector<DirectionalLight*> directionalLights;

	// RTV, SRV, Text
	ID3D11Texture2D* lightText;
	ID3D11RenderTargetView* lightRTV;
	ID3D11ShaderResourceView* lightSRV;

	// Mesh associated with lights
	Mesh* pointLightMesh;
	Mesh* spotLightMesh;

	// All light pixel shaders
	SimplePixelShader* pointLightPS;
	SimplePixelShader* directionalLightPS;

	// Basic mesh vertex shader and fullscreen quad
	SimpleVertexShader* lightVS;
	SimpleVertexShader* quadVS; // taken from renderer
};

