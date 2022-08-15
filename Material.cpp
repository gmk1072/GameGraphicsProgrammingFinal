#include "Material.h"
#include "MemoryDebug.h"

// Set material ID start to 0
unsigned int Material::staticMaterialID = 0;

// --------------------------------------------------------
// Constructor
//
// Creates a basic material with supplied vertex and pixel
// shader (required for basic drawing). Also prepares info
// for texture drawing.
//
// vertexShader - Vertex shader to use when drawing.
// pixelShader - Pixel shader to use when drawing.
// texture2D - Texture to use when drawing.
// --------------------------------------------------------
Material::Material(SimpleVertexShader * const vertexShader,
	SimplePixelShader * const pixelShader,
	Texture2D* const texture2D) :
	vertexShader(vertexShader),
	pixelShader(pixelShader),
	stencilID(1)
{
	// initialize single texture
	textureList.numTextures = 1;
	textureList.textures = new Texture2D*;
	textureList.textures[0] = texture2D;

	// assigned next ID
	materialID = staticMaterialID++;
}

// --------------------------------------------------------
// Constructor
//
// Creates a material that will utilize a normal texture.
//
// vertexShader - Vertex shader to use when drawing.
// pixelShader - Pixel shader to use when drawing.
// albedoTexture - Texture to use when drawing.
// normalTexture - Texture to sample normals from when drawing.
// --------------------------------------------------------
Material::Material(SimpleVertexShader * const vertexShader,
	SimplePixelShader * const pixelShader,
	Texture2D* const albedoTexture,
	Texture2D* const normalTexture) :
	vertexShader(vertexShader),
	pixelShader(pixelShader),
	stencilID(1)
{
	// Save multiple textures
	textureList.numTextures = 2;
	textureList.textures = new Texture2D*[2];
	
	// copy pointers
	textureList.textures[0] = albedoTexture;
	textureList.textures[1] = normalTexture;

	// assigned next ID
	materialID = staticMaterialID++;
}

// --------------------------------------------------------
// Constructor
//
// Creates a material that will utilize a normal texture and
// emission texture.
//
// vertexShader - Vertex shader to use when drawing.
// pixelShader - Pixel shader to use when drawing.
// albedoTexture - Texture to use when drawing.
// normalTexture - Texture to sample normals from when drawing.
// emissionTexture - Texture to sample light from when lighting.
// --------------------------------------------------------
Material::Material(SimpleVertexShader * const vertexShader, 
	SimplePixelShader * const pixelShader,
	Texture2D * const albedoTexture, 
	Texture2D * const normalTexture,
	Texture2D * const emissionTexture) :
	vertexShader(vertexShader),
	pixelShader(pixelShader),
	stencilID(1)
{
	// Save multiple textures
	textureList.numTextures = 3;
	textureList.textures = new Texture2D*[3];

	// copy pointers
	textureList.textures[0] = albedoTexture;
	textureList.textures[1] = normalTexture;
	textureList.textures[2] = emissionTexture;

	// assigned next ID
	materialID = staticMaterialID++;
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
Material::~Material()
{
	// Free double ptr array of textures
	if(textureList.textures)
		delete[] textureList.textures;
}

// --------------------------------------------------------
// Call this from the renderer such that textures and other
// relevant material specific information is set.
// --------------------------------------------------------
void Material::PrepareMaterial()
{
	// Loop through bounds textures and set them as active
	Texture2D* currTexture;
	for (size_t i = textureList.numTextures; i--;)
	{
		currTexture = textureList.textures[i];
		pixelShader->SetShaderResourceView(currTexture->GetSRVName(), currTexture->GetSRV());
		pixelShader->SetSamplerState(currTexture->GetSampelerName(), currTexture->GetSamplerState());
	}

	// Set up additional shader information (if provided)
	// IF I WERE A SPECULAR_MATERIAL
	// SEND UP CAMERA LOCATION
}

// --------------------------------------------------------
// Get the vertex shader for this material
// --------------------------------------------------------
SimpleVertexShader * const Material::GetVertexShader() const
{
	return vertexShader;
}

// --------------------------------------------------------
// Get the pixel shader for this material
// --------------------------------------------------------
SimplePixelShader * const Material::GetPixelShader() const
{
	return pixelShader;
}

// --------------------------------------------------------
// Get the texture information for this material
// --------------------------------------------------------
Texture2D * const Material::GetTexture2D(size_t index) const
{
	if (!textureList.textures)
		return nullptr;

	if (index >= textureList.numTextures)
		return nullptr;

	return textureList.textures[index];
}

// --------------------------------------------------------
// Get the material ID for this material
// --------------------------------------------------------
unsigned int Material::GetID() const
{
	return materialID;
}


