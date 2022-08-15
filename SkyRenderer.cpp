#include "SkyRenderer.h"
#include "MemoryDebug.h"

SkyRenderer::SkyRenderer(Renderer & renderer) :
	renderer(renderer)
{
	device = renderer.device;
	context = renderer.context;
}


SkyRenderer::~SkyRenderer()
{
	if (skySRV) skySRV->Release();
	if (skySampler) skySampler->Release();
	if (skyRasterizer)  skyRasterizer->Release();
	if (skyDepthStencil) skyDepthStencil->Release();
	if (skyVS) delete skyVS;
	if (skyPS) delete skyPS;
	if (skyMesh) delete skyMesh;
}

HRESULT SkyRenderer::init()
{
	// Load Shaders
	HRESULT result = loadShaders();
	if (result != S_OK)
		return E_FAIL;

	// Load Assets
	result = loadAssets();
	if (result != S_OK)
		return E_FAIL;


	// Create a sampler for sampling the skytexture
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &skySampler);

	// Set up the rasterizer state for the sky
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;	// I am not sure which way the normals are facing
	rasterizerDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rasterizerDesc, &skyRasterizer);

	// Used to figure out what pixels to draw the sky on.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthStencilDesc, &skyDepthStencil);

	return S_OK;
}

// Loads shaders
HRESULT SkyRenderer::loadShaders()
{
	// Load Vertex Shader -	Used to define the mesh the sky is drawn onto
	skyVS = renderer.CreateSimpleVertexShader();
	if (!skyVS->LoadShaderFile(L"./Assets/Shaders/SkyVS.cso")) {
		return E_FAIL;
	}

	// Load Pixel Shader -	Used to draw the sky onto the mesh
	skyPS = renderer.CreateSimplePixelShader();
	if (!skyPS->LoadShaderFile(L"./Assets/Shaders/SkyPS.cso")) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT SkyRenderer::loadAssets()
{
	ID3D11Device* device = renderer.device;
	ID3D11DeviceContext* context = renderer.context;

	// Load Mesh
	char skyMeshPath[] = "./Assets/Models/cube.obj";
	skyMesh = renderer.CreateMesh(skyMeshPath);
	if (!skyMesh) {
		fprintf(stderr, "[Skybox] Failed to load sky mesh %s\n", skyMeshPath);
		return E_FAIL;
	}

	// Load the skybox and store it as a texture cube under the hood
	wchar_t skyTexturePath[] = L"./Assets/Textures/starscape.dds";
	if (CreateDDSTextureFromFile(device, skyTexturePath, 0, &skySRV) != S_OK) {
		fprintf(stderr, "[Skybox] Failed to load sky texture %ls\n", skyTexturePath);
		return E_FAIL;
	}

	return S_OK;
}

void SkyRenderer::Render(const Camera * const camera)
{
	XMFLOAT4X4 view = camera->GetViewMatrix();
	XMFLOAT4X4 projection = camera->GetProjectionMatrix();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* skyVB = skyMesh->GetVertexBuffer();
	ID3D11Buffer* skyIB = skyMesh->GetIndexBuffer();
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Update sky vertex shader
	skyVS->SetMatrix4x4("view", view);
	skyVS->SetMatrix4x4("projection", projection);
	skyVS->CopyAllBufferData();
	skyVS->SetShader();

	// Update sky particle shader
	skyPS->SetShaderResourceView("Skybox", skySRV);
	skyPS->SetSamplerState("Sampler", skySampler);
	skyPS->SetFloat4("tint", XMFLOAT4(1, 1, 1, 1));
	skyPS->CopyAllBufferData();
	skyPS->SetShader();

	context->RSSetState(skyRasterizer);
	context->OMSetDepthStencilState(skyDepthStencil, 0);
	context->DrawIndexed(skyMesh->GetIndexCount(), 0, 0);

	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}
