#pragma once

#include <d3d11.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <stdio.h>

enum class Texture2DType
{
	ALBEDO,
	NORMAL,
	EMISSION,
	PARALLAX
};

enum class Texture2DFileType
{
	OTHER,
	DDS
};

class Texture2D
{
public:
	Texture2D(
		const wchar_t* filePath,								// Path to texture 2D
		Texture2DType type,										// Texture type
		Texture2DFileType fileType,								// File type of texture, other by default
		ID3D11SamplerState* const samplerState,					// Sampler state to use for this texture
		ID3D11Device* const device,								// GFX device used to create texture
		ID3D11DeviceContext* const context						// GFX context used to create texture
	);
	~Texture2D();

	// Getters for sampler state and resource view
	ID3D11SamplerState* const GetSamplerState() const;
	ID3D11ShaderResourceView* const GetSRV() const;
	const char* const GetSampelerName() const;
	const char* const GetSRVName() const;

private:
	// Texture information to send to shader
	ID3D11SamplerState* samplerState;
	ID3D11ShaderResourceView* srv;

	const char* samplerName;
	const char* srvName;

};

