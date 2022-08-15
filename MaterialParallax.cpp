#include "MaterialParallax.h"


void MaterialParallax::PrepareMaterial()
{
	Material::PrepareMaterial();

	XMFLOAT4X4 mat = camera->GetViewMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mat);
	XMVECTOR s,r,v;
	XMMatrixDecompose(&s, &r, &v, view);
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, v);
	vertexShader->SetFloat3("viewPos", pos);
}

MaterialParallax::MaterialParallax(SimpleVertexShader * const vertexShader,
	SimplePixelShader * const pixelShader,
	Texture2D * const albedoTexture,
	Texture2D * const normalTexture,
	Texture2D * const parallaxTexture,
	Camera * camera) :
	Material(vertexShader, pixelShader, albedoTexture, normalTexture, parallaxTexture)
{
	this->camera = camera;
}


MaterialParallax::~MaterialParallax()
{
}
