#include "MaterialEnemy.h"



MaterialEnemy::MaterialEnemy(
	SimpleVertexShader * const vertexShader,
	SimplePixelShader * const pixelShader,
	Texture2D * const albedoTexture,
	Texture2D * const normalTexture,
	Texture2D * const emissionTexture) :
	Material(vertexShader, pixelShader, albedoTexture, normalTexture, emissionTexture)
{
	totalTime = 0;
	timeOffset = (rand() % 10000) / 10000.0f;
}

MaterialEnemy::~MaterialEnemy()
{
}

void MaterialEnemy::PrepareMaterial()
{
	Material::PrepareMaterial();

	// Get albedo texture
	Texture2D* texture = textureList.textures[0];

	// Use albedo texture in vertex shader
	vertexShader->SetShaderResourceView("Texture", texture->GetSRV());
	vertexShader->SetSamplerState("Sampler", texture->GetSamplerState());

	// Set time in vertex shader
	vertexShader->SetFloat("time", totalTime + timeOffset);
}

void MaterialEnemy::SetTotalTime(float totalTime)
{
	this->totalTime = totalTime;
}
