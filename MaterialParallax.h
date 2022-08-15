#pragma once
#include "Material.h"
#include "CameraGame.h"
class MaterialParallax :
	public Material
{
public:
	/*
	MaterialParallax(
		SimpleVertexShader * const vertexShader,	// Required shader needed to draw stuff
		SimplePixelShader * const pixelShader,		// Required shader needed to draw stuff
		Texture2D * const albedoTexture,			// albedo texture
		Texture2D * const normalTexture,			// normal texture
		Texture2D * const parallaxTexture,			// emission texture
		Camera * camera
	);*/
	~MaterialParallax();

	void PrepareMaterial() override;
	MaterialParallax(SimpleVertexShader * const vertexShader, SimplePixelShader * const pixelShader, Texture2D * const albedoTexture, Texture2D * const normalTexture, Texture2D * const parallaxTexture, Camera * camera);
private:
	Camera* camera;
};

