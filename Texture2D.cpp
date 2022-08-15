#include "Texture2D.h"
#include "MemoryDebug.h"

// --------------------------------------------------------
// Constructor
//
// Creates a basic texture from a given filepath and 
// provided sampler state.
// --------------------------------------------------------
Texture2D::Texture2D(const wchar_t * filePath,
	Texture2DType type,
	Texture2DFileType fileType,
	ID3D11SamplerState * const samplerState,
	ID3D11Device * const device,
	ID3D11DeviceContext * const context) :
	samplerState(samplerState)
{
	switch (fileType)
	{
	case Texture2DFileType::OTHER:
		// Load a texture
		if (DirectX::CreateWICTextureFromFile(
			device,
			context,
			filePath,
			0, // we dont need the texture ref itself
			&srv
		) != S_OK)
			fprintf(stderr, "[Texture2D] Failed to load texture %ls\n", filePath);
		break;
	case Texture2DFileType::DDS:
		// Load a texture
		if (DirectX::CreateDDSTextureFromFile(
			device,
			context,
			filePath,
			0, // we dont need the texture ref itself
			&srv
		) != S_OK)
			fprintf(stderr, "[Texture2D] Failed to load texture %ls\n", filePath);
		break;
	default:
		break;
	}


	// Set texture type information
	switch (type)
	{
		case Texture2DType::ALBEDO:
			samplerName = "albedoSampler";
			srvName = "albedo";
			break;
		case Texture2DType::NORMAL:
			samplerName = "albedoSampler";
			srvName = "normalMap";
			break;
		case Texture2DType::EMISSION:
			samplerName = "albedoSampler";
			srvName = "emissionMap";
			break;
		case Texture2DType::PARALLAX:
			samplerName = "albedoSampler";
			srvName = "depthMap";
			break;
		default:
			// ASSERT?
			break;
	}
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
Texture2D::~Texture2D()
{
	//this check because failed textures are nullptr
	if( srv != nullptr) srv->Release();
}

// --------------------------------------------------------
// Get the sampler state for this texture
// --------------------------------------------------------
ID3D11SamplerState * const Texture2D::GetSamplerState() const
{
	return samplerState;
}

// --------------------------------------------------------
// Get the shader resource view for this texture
// --------------------------------------------------------
ID3D11ShaderResourceView * const Texture2D::GetSRV() const
{
	return srv;
}

// --------------------------------------------------------
// Get the name of this texture's sampler
// --------------------------------------------------------
const char * const Texture2D::GetSampelerName() const
{
	return samplerName;
}

// --------------------------------------------------------
// Get the name of this texture's shader resource view
// --------------------------------------------------------
const char * const Texture2D::GetSRVName() const
{
	return srvName;
}
