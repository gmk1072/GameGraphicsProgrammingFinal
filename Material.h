#pragma once

#include "SimpleShader.h"
#include "Texture2D.h"

class Material
{
public:
	Material(
		SimpleVertexShader* const vertexShader,		// Required shader needed to draw stuff
		SimplePixelShader* const pixelShader,		// Required shader needed to draw stuff
		Texture2D* const texture2D					// Texture to use for this material
	);

	Material(
		SimpleVertexShader * const vertexShader,	// Required shader needed to draw stuff
		SimplePixelShader * const pixelShader,		// Required shader needed to draw stuff
		Texture2D * const albedoTexture,			// albedo texture
		Texture2D * const normalTexture				// normal texture
	);

	Material(
		SimpleVertexShader * const vertexShader,	// Required shader needed to draw stuff
		SimplePixelShader * const pixelShader,		// Required shader needed to draw stuff
		Texture2D * const albedoTexture,			// albedo texture
		Texture2D * const normalTexture,			// normal texture
		Texture2D * const emissionTexture			// emission texture
		);

	~Material();

	// Set vertex/pixel shader information
	virtual void PrepareMaterial();

	// Getters for shader types
	SimpleVertexShader* const GetVertexShader() const;
	SimplePixelShader* const GetPixelShader() const;

	// Getters for sampler state and resource view
	Texture2D* const GetTexture2D(size_t index) const;
	unsigned int GetID() const;

	// Stencil ID for this material
	unsigned int stencilID;

protected:
	struct Textures
	{
		Texture2D** textures;
		size_t numTextures;
	};

	// Used for unique identification of materials for the Renderer
	static unsigned int staticMaterialID;

	// Pointers to required shaders needed to draw stuff
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Pointers to info needed to attach a texture
	Textures textureList;

	// Material ID for this material
	unsigned int materialID;
};

