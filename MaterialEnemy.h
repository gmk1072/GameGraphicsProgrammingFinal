#pragma once
#include "Material.h"

// Special material used be enemy
class MaterialEnemy :
	public Material
{
public:
	MaterialEnemy(
		SimpleVertexShader * const vertexShader,	// Required shader needed to draw stuff
		SimplePixelShader * const pixelShader,		// Required shader needed to draw stuff
		Texture2D * const albedoTexture,			// albedo texture
		Texture2D * const normalTexture,			// normal texture
		Texture2D * const emissionTexture			// emission texture
		);
	~MaterialEnemy();

	// Prepare the material to include time
	void PrepareMaterial() override;

	// Set time used by material
	void SetTotalTime(float totalTime);

private:
	float totalTime, timeOffset;
};

