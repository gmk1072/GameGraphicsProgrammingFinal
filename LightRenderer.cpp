#include "LightRenderer.h"

// --------------------------------------------------------
// Constructor
//
// Creates a light renderer which will use the context and
// device found in the given renderer.
//
// renderer - Reference to renderer object in which to render
//			  light to.
// --------------------------------------------------------
LightRenderer::LightRenderer(Renderer & renderer) :
	renderer(renderer),
	lights(32),
	lightText(nullptr),
	lightRTV(nullptr),
	lightSRV(nullptr),
	pointLightMesh(nullptr),
	spotLightMesh(nullptr),
	pointLightPS(nullptr),
	directionalLightPS(nullptr),
	lightVS(nullptr),
	quadVS(nullptr)

{
	pointLights.reserve(16);
	directionalLights.reserve(16);
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
LightRenderer::~LightRenderer()
{
}

// --------------------------------------------------------
// Initializes the light renderer.
//
// quadVS - The vertex shader which draws a full screen quad.
// width - Width of screen.
// height - Height of screen.
//
// returns - Result of initialization.
// --------------------------------------------------------
HRESULT LightRenderer::Initialize(SimpleVertexShader* quadVS, unsigned int width, unsigned int height)
{
	HRESULT hr;

	// Create texture, rtv, srv
	D3D11_TEXTURE2D_DESC renderTargetTextDesc = {};
	renderTargetTextDesc.Width = width;
	renderTargetTextDesc.Height = height;
	renderTargetTextDesc.MipLevels = 1;
	renderTargetTextDesc.ArraySize = 1;
	renderTargetTextDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetTextDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextDesc.CPUAccessFlags = 0;
	renderTargetTextDesc.MiscFlags = 0;
	renderTargetTextDesc.SampleDesc.Count = 1;
	renderTargetTextDesc.SampleDesc.Quality = 0;
	hr = renderer.device->CreateTexture2D(&renderTargetTextDesc, nullptr, &lightText);
	if (FAILED(hr))
		return hr;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = renderTargetTextDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	hr = renderer.device->CreateRenderTargetView(lightText, &renderTargetViewDesc, &lightRTV);
	if (FAILED(hr))
		return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC renderTargetSRVDesc = {};
	renderTargetSRVDesc.Format = renderTargetTextDesc.Format;
	renderTargetSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	renderTargetSRVDesc.Texture2D.MipLevels = 1;
	renderTargetSRVDesc.Texture2D.MostDetailedMip = 0;
	hr = renderer.device->CreateShaderResourceView(lightText, &renderTargetSRVDesc, &lightSRV);
	if (FAILED(hr))
		return hr;

	// Load point light mesg
	// TODO: loading a mesh in twice, not good...
	pointLightMesh = renderer.CreateMesh("./Assets/Models/sphere.obj");
	if (!pointLightMesh)
		return E_FAIL;

	// Load shaders
	pointLightPS = renderer.CreateSimplePixelShader();
	if (!pointLightPS->LoadShaderFile(L"./Assets/Shaders/DeferredPointLightPS.cso"))
		return E_FAIL;

	directionalLightPS = renderer.CreateSimplePixelShader();
	if (!directionalLightPS->LoadShaderFile(L"./Assets/Shaders/DeferredDirectionalLightPS.cso"))
		return E_FAIL;

	lightVS = renderer.CreateSimpleVertexShader();
	if (!lightVS->LoadShaderFile(L"./Assets/Shaders/DeferredLightVS.cso"))
		return E_FAIL;

	if (!(this->quadVS = quadVS))
		return E_FAIL;

	return S_OK;
}

// --------------------------------------------------------
// Cleans up and shutsdown this light renderer object.
//
// returns - Result of shutdown.
// --------------------------------------------------------
HRESULT LightRenderer::Shutdown()
{
	// Free all lights
	for (auto it = lights.cbegin(); it != lights.cend(); it++)
		if(it->second)
			delete it->second;

	// Free text,rtv,srv
	if (lightText) { lightText->Release(); }
	if (lightRTV) { lightRTV->Release(); }
	if (lightSRV) { lightSRV->Release(); }

	// Free meshs
	if (pointLightMesh) { delete pointLightMesh; }

	// Free Shaders
	if (pointLightPS) { delete pointLightPS; }
	if (directionalLightPS) { delete directionalLightPS; }
	if (lightVS) { delete lightVS; }
	// Do not free QuadVS, that is handled by the renderer.

	return S_OK;
}

// --------------------------------------------------------
// Creates a point light object.
//
// NOTE: The name of the point light must not already exist
//		 in the light renderer!
//
// attenuation - Attenuation type for this point light.
// name - Unique name for this point light.
// autostage - Whether or not the light should be staged for 
//			   rendering on creation.
//
// return - Pointer to point light object.
// --------------------------------------------------------
PointLight * const LightRenderer::CreatePointLight(PointLightAttenuation attenuation, std::string name, bool autostage)
{
	// Ensure name doesnt already exist
	assert(lights.count(name) == 0);
	PointLight* pointLight = new PointLight(attenuation);
	lights[name] = pointLight;
	if (autostage)
		pointLights.push_back(pointLight);
	return pointLight;
}

// --------------------------------------------------------
// Creates a directional light object.
//
// NOTE: The name of the directional light must not already exist
//		 in the light renderer!
//
// name - Unique name for this directional light.
// autostage - Whether or not the light should be staged for 
//			   rendering on creation.
//
// return - Pointer to directional light object.
// --------------------------------------------------------
DirectionalLight * const LightRenderer::CreateDirectionalLight(std::string name, bool autostage)
{
	// Ensure name doesnt already exist
	assert(lights.count(name) == 0);
	DirectionalLight* directionalLight = new DirectionalLight();
	lights[name] = directionalLight;
	if (autostage)
		directionalLights.push_back(directionalLight);
	return directionalLight;
}

// --------------------------------------------------------
// Renders all stages lights to this light renderer's light
// RTV according to depth found in the renderer's 
// depth stencil view.
//
// NOTE: D3D11 state values are not preserved.
//
// camera - Camera to use when rendering lights.
// --------------------------------------------------------
void LightRenderer::Render(const Camera * const camera)
{
	//renderer.depthStencilView
	// Set render target
	renderer.context->OMSetRenderTargets(1, &lightRTV, renderer.depthStencilView);
	renderer.context->OMSetDepthStencilState(renderer.lightStencilState, 0);

	// Mesh related information
	const Mesh* currMesh = nullptr;
	ID3D11Buffer* currVertBuff = nullptr;
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;

	// Set once vertex shader information
	lightVS->SetMatrix4x4("view", camera->GetViewMatrix());
	lightVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	// Iterate through all point lights
	// Set SRVs and other const information once
	pointLightPS->SetShaderResourceView("worldPosTexture", renderer.targetSRVs[1]);
	pointLightPS->SetShaderResourceView("normalsTexture", renderer.targetSRVs[2]);
	pointLightPS->SetSamplerState("deferredSampler", renderer.targetSampler);
	pointLightPS->SetFloat2("screenSize", XMFLOAT2(renderer.viewport.Width, renderer.viewport.Height));

	// Setup mesh information
	currMesh = pointLightMesh;
	currVertBuff = pointLightMesh->GetVertexBuffer();
	renderer.context->IASetVertexBuffers(0, 1, &currVertBuff, &stride, &offset);
	renderer.context->IASetIndexBuffer(currMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	for (auto it = pointLights.cbegin(); it != pointLights.cend(); it++)
	{
		PointLight* const currPL = (*it);
		
		// Update info...
		currPL->PrepareForShader();

		// set position
		lightVS->SetMatrix4x4("world", currPL->transform.GetWorldMatrix());
		
		// set light specific
		pointLightPS->SetStruct("diffuse", &currPL->pointLightLayout, sizeof(PointLightLayout));

		// upload shader properties
		pointLightPS->CopyAllBufferData();
		lightVS->CopyAllBufferData();
		pointLightPS->SetShader();
		lightVS->SetShader();

		// draw
		renderer.context->DrawIndexed(currMesh->GetIndexCount(), 0, 0);
	}

	// Iterate through all spot lights


	// Ensure zbuffer off
	renderer.context->OMSetDepthStencilState(nullptr, 0);

	// Iterate through all directional lights
	// Setup mesh information
	renderer.context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	renderer.context->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);

	// Set SRVs and other const information once
	directionalLightPS->SetShaderResourceView("worldPosTexture", renderer.targetSRVs[1]);
	directionalLightPS->SetShaderResourceView("normalsTexture", renderer.targetSRVs[2]);
	directionalLightPS->SetSamplerState("deferredSampler", renderer.targetSampler);

	// Set quad VS once
	quadVS->SetShader();

	for (auto it = directionalLights.cbegin(); it != directionalLights.cend(); it++)
	{
		DirectionalLight* const currDL = (*it);

		// set light specific
		directionalLightPS->SetStruct("diffuse", &currDL->directionalLightLayout, sizeof(DirectionalLightLayout));

		// upload shader properties
		directionalLightPS->CopyAllBufferData();
		directionalLightPS->SetShader();

		// draw quad
		renderer.context->Draw(3, 0);
	}
}

// --------------------------------------------------------
// Clears the light RTV.
// --------------------------------------------------------
void LightRenderer::ClearRTV()
{
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderer.context->ClearRenderTargetView(lightRTV, color);
}
