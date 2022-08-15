#include "Renderer.h"
#include "MemoryDebug.h"

// Initialize instance to null
Renderer* Renderer::instance = nullptr;

Renderer::Renderer(DXWindow* const window)
{
	HRESULT ret;

	// Set initial number of buckets and stuff
	renderBatches.rehash(16);
	renderBatches.reserve(16);

	// Init DXCore
	ret = InitDirectX(window);
	if (ret != S_OK)
		fprintf(stderr, "[Renderer] Failed to initialize DXCore\n");

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Initialize UI stuff
	spriteBatch = new SpriteBatch(context);
	panel = nullptr;

	//Set data using window size etc.
	texelWidth = 1.0f / window->GetWidth();
	texelHeight = 1.0f / window->GetHeight();
	blurDist = 4;
	glowDist = 50;
	colorThreshold = .2f;
	glowPercentage = .5f;
}

// --------------------------------------------------------
// Destructor - Clean up (release) all DirectX references
// --------------------------------------------------------
Renderer::~Renderer()
{
	// Free fonts
	for (auto it = fontMap.begin(); it != fontMap.end(); it++)
		if(it->second)
			delete it->second;

	// Free spritebatch
	if (spriteBatch) { delete spriteBatch; }

	// Release all DirectX resources
	// Release targets
	for (size_t i = 0; i < BUFFER_COUNT; i++)
	{
		if (targetTexts[i])
			targetTexts[i]->Release();
		if (targetViews[i])
			targetViews[i]->Release();
		if (targetSRVs[i])
			targetSRVs[i]->Release();
	}

	for (size_t i = 0; i < 3; i++)
	{
		if (postProcessTexts[i])
			postProcessTexts[i]->Release();
		if (postProcessRTVs[i])
			postProcessRTVs[i]->Release();
		if (postProcessSRVs[i])
			postProcessSRVs[i]->Release();
	}

	for (size_t i = 0; i < 2; i++)
	{
		if (halfTexts[i])
			halfTexts[i]->Release();
		if (halfRTVs[i])
			halfRTVs[i]->Release();
		if (halfSRVs[i])
			halfSRVs[i]->Release();
	}

	// DX11 release only
	// Free sampler state which is being used for all textures
	if (objectTextureSampler) { objectTextureSampler->Release(); };
	if (targetSampler) { targetSampler->Release(); }
	if (depthStencilView) { depthStencilView->Release(); }
	if (depthBufferTexture) { depthBufferTexture->Release(); }
	if (depthStencilState) { depthStencilState->Release(); }
	if (lightStencilState) { lightStencilState->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }
	if (depthSRV) { depthSRV->Release(); }
	if (swapChain) { swapChain->Release(); }
	if (addBlendState) { addBlendState->Release(); }
	if (context) { context->Release(); }
	if (device) { device->Release(); }

	// Shaders
	if (deferredVS) { delete deferredVS; }
	if (deferredLightingPS) { delete deferredLightingPS; }
	if (volumetricLightingPS) { delete volumetricLightingPS; }
	if (downsamplePS) { delete downsamplePS; }
	if (upsamplePS) { delete upsamplePS; }
	if (horizontalBlurPS) { delete horizontalBlurPS; }
	if (verticalBlurPS) { delete verticalBlurPS; }
	if (postPS) { delete postPS; }
	if (deferredLightVS) { delete deferredLightVS; }
	if (prePostProcessPS) { delete prePostProcessPS; }

	// Particle renderer
	if (particleRenderer) { particleRenderer->Shutdown();  delete particleRenderer; }

	// Sky renderer
	if (skyRenderer) { delete skyRenderer; }

	// Light renderer
	if (lightRenderer) { lightRenderer->Shutdown(); delete lightRenderer; }
}

// --------------------------------------------------------
// Initializes DirectX, which requires a window.  This method
// also creates several DirectX objects we'll need to start
// drawing things to the screen.
// TO-DO: COMMENT REST OF FUNCTIONS
//
// window - reference to window to draw to
// --------------------------------------------------------
HRESULT Renderer::InitDirectX(DXWindow* const window)
{
	// This will hold options for DirectX initialization
	unsigned int deviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// If we're in debug mode in visual studio, we also
	// want to make a "Debug DirectX Device" to see some
	// errors and warnings in Visual Studio's output window
	// when things go wrong!
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create a description of how our swap
	// chain should work
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = window->GetWidth();
	swapDesc.BufferDesc.Height = window->GetHeight();
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = window->GetWindowReference();
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;

	// Result variable for below function calls
	HRESULT hr = S_OK;

	// Attempt to initialize DirectX
	hr = D3D11CreateDeviceAndSwapChain(
		0,							// Video adapter (physical GPU) to use, or null for default
		D3D_DRIVER_TYPE_HARDWARE,	// We want to use the hardware (GPU)
		0,							// Used when doing software rendering
		deviceFlags,				// Any special options
		0,							// Optional array of possible verisons we want as fallbacks
		0,							// The number of fallbacks in the above param
		D3D11_SDK_VERSION,			// Current version of the SDK
		&swapDesc,					// Address of swap chain options
		&swapChain,					// Pointer to our Swap Chain pointer
		&device,					// Pointer to our Device pointer
		&dxFeatureLevel,			// This will hold the actual feature level the app will use
		&context);					// Pointer to our Device Context pointer
	if (FAILED(hr)) return hr;

	// The above function created the back buffer render target
	// for us, but we need a reference to it
	ID3D11Texture2D* backBufferTexture;
	swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBufferTexture);

	// Now that we have the texture, create a render target view
	// for the back buffer so we can render into it.  Then release
	// our local reference to the texture, since we have the view.
	device->CreateRenderTargetView(
		backBufferTexture,
		0,
		&backBufferRTV);
	backBufferTexture->Release();

	// Set up the description of the texture to use for the depth buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = window->GetWidth();
	depthStencilDesc.Height = window->GetHeight();
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);

	// Set up the depth stencil view description.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthBufferTexture, &depthStencilViewDesc, &depthStencilView);

	// Lastly, set up a viewport so we render into
	// to correct portion of the window
	viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)window->GetWidth();
	viewport.Height = (float)window->GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	halfViewport = {};
	halfViewport.TopLeftX = 0;
	halfViewport.TopLeftY = 0;
	halfViewport.Width = (float)window->GetWidth()/2;
	halfViewport.Height = (float)window->GetHeight()/2;
	halfViewport.MinDepth = 0.0f;
	halfViewport.MaxDepth = 1.0f;

	// Setup render target texture descriptions
	D3D11_TEXTURE2D_DESC renderTargetTextDesc = {};
	renderTargetTextDesc.Width = window->GetWidth();
	renderTargetTextDesc.Height = window->GetHeight();
	renderTargetTextDesc.MipLevels = 1;
	renderTargetTextDesc.ArraySize = 1;
	renderTargetTextDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetTextDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextDesc.CPUAccessFlags = 0;
	renderTargetTextDesc.MiscFlags = 0;
	renderTargetTextDesc.SampleDesc.Count = 1;
	renderTargetTextDesc.SampleDesc.Quality = 0;

	D3D11_TEXTURE2D_DESC halfTargetTextDesc = {};
	halfTargetTextDesc.Width = window->GetWidth()/2;
	halfTargetTextDesc.Height = window->GetHeight()/2;
	halfTargetTextDesc.MipLevels = 1;
	halfTargetTextDesc.ArraySize = 1;
	halfTargetTextDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	halfTargetTextDesc.Usage = D3D11_USAGE_DEFAULT;
	halfTargetTextDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	halfTargetTextDesc.CPUAccessFlags = 0;
	halfTargetTextDesc.MiscFlags = 0;
	halfTargetTextDesc.SampleDesc.Count = 1;
	halfTargetTextDesc.SampleDesc.Quality = 0;

	// Create render target textures
	for (unsigned int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = device->CreateTexture2D(&renderTargetTextDesc, nullptr, &targetTexts[i]);
		if (FAILED(hr))
			return hr;
	}

	for (unsigned int i = 0; i < 3; i++)
	{
		hr = device->CreateTexture2D(&renderTargetTextDesc, nullptr, &postProcessTexts[i]);
		if (FAILED(hr))
			return hr;
	}

	for (unsigned int i = 0; i < 2; i++)
	{
		hr = device->CreateTexture2D(&halfTargetTextDesc, nullptr, &halfTexts[i]);
		if (FAILED(hr))
			return hr;
	}

	// Setup render target view description
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = renderTargetTextDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	D3D11_RENDER_TARGET_VIEW_DESC halfTargetViewDesc = {};
	halfTargetViewDesc.Format = halfTargetTextDesc.Format;
	halfTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	halfTargetViewDesc.Texture2D.MipSlice = 0;

	// Create render target views
	for (unsigned int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = device->CreateRenderTargetView(targetTexts[i], &renderTargetViewDesc, &targetViews[i]);
		if (FAILED(hr))
			return hr;
	}

	for (unsigned int i = 0; i < 3; i++)
	{
		hr = device->CreateRenderTargetView(postProcessTexts[i], &renderTargetViewDesc, &postProcessRTVs[i]);
		if (FAILED(hr))
			return hr;
	}

	for (unsigned int i = 0; i < 2; i++)
	{
		hr = device->CreateRenderTargetView(halfTexts[i], &halfTargetViewDesc, &halfRTVs[i]);
		if (FAILED(hr))
			return hr;
	}

	// Setup shader resource view descriptions for render targets
	D3D11_SHADER_RESOURCE_VIEW_DESC renderTargetSRVDesc = {};
	renderTargetSRVDesc.Format = renderTargetTextDesc.Format;
	renderTargetSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	renderTargetSRVDesc.Texture2D.MipLevels = 1;
	renderTargetSRVDesc.Texture2D.MostDetailedMip = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC halfTargetSRVDesc = {};
	halfTargetSRVDesc.Format = halfTargetTextDesc.Format;
	halfTargetSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	halfTargetSRVDesc.Texture2D.MipLevels = 1;
	halfTargetSRVDesc.Texture2D.MostDetailedMip = 0;

	// Create SRVs for targets
	for (unsigned int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = device->CreateShaderResourceView(targetTexts[i], &renderTargetSRVDesc, &targetSRVs[i]);
		if (FAILED(hr))
			return hr;
	}

	for (unsigned int i = 0; i < 3; i++)
	{
		hr = device->CreateShaderResourceView(postProcessTexts[i], &renderTargetSRVDesc, &postProcessSRVs[i]);
		if (FAILED(hr))
			return hr;
	}

	for (unsigned int i = 0; i < 2; i++)
	{
		hr = device->CreateShaderResourceView(halfTexts[i], &halfTargetSRVDesc, &halfSRVs[i]);
		if (FAILED(hr))
			return hr;
	}
	renderTargetSRVDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
	hr = device->CreateShaderResourceView(depthBufferTexture, &renderTargetSRVDesc, &depthSRV);
	if (FAILED(hr))
		return hr;

	// Setup add blend state
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(&blendDesc, &addBlendState);
	if (FAILED(hr))
		return hr;

	// Setup deferred sampler state
	D3D11_SAMPLER_DESC targetSamplerDesc = {}; // inits to all zeros :D!
	targetSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	targetSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	targetSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	targetSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	targetSamplerDesc.MipLODBias = 0.0f;
	targetSamplerDesc.MaxAnisotropy = 1;
	targetSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	targetSamplerDesc.BorderColor[0] = 0;
	targetSamplerDesc.BorderColor[1] = 0;
	targetSamplerDesc.BorderColor[2] = 0;
	targetSamplerDesc.BorderColor[3] = 0;
	targetSamplerDesc.MinLOD = 0;
	targetSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	hr = device->CreateSamplerState(&targetSamplerDesc, &targetSampler);
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_DESC depthState = {};
	depthState.DepthEnable = true;
	depthState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthState.DepthFunc = D3D11_COMPARISON_LESS;
	depthState.StencilEnable = true;
	depthState.StencilReadMask = 0xFF;
	depthState.StencilWriteMask = 0xFF;
	depthState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthState.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	depthState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
	depthState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthState.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&depthState, &depthStencilState);
	if (FAILED(hr))
		return hr;
	
	depthState.DepthEnable = false;
	depthState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	depthState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthState.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	hr = device->CreateDepthStencilState(&depthState, &lightStencilState);
	if (FAILED(hr))
		return hr;

	// load deferred lighting and vert shader
	deferredLightingPS = CreateSimplePixelShader();
	if (!deferredLightingPS->LoadShaderFile(L"./Assets/Shaders/DefferedLighting.cso"))
		return E_FAIL;

	deferredVS = CreateSimpleVertexShader();
	if (!deferredVS->LoadShaderFile(L"./Assets/Shaders/Texture2BufferVertexShader.cso"))
		return E_FAIL;

	// Setup texture stuff
	// Create a sampler state
	D3D11_SAMPLER_DESC sampDesc = {}; // inits to all zeros :D!
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // wrap in all dirs
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Trilinear
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, &objectTextureSampler);

	//================================== Post-processing Stuff ====================================
	volumetricLightingPS = CreateSimplePixelShader();
	if (!volumetricLightingPS->LoadShaderFile(L"./Assets/Shaders/VolumetricLightingPixelShader.cso"))
		return E_FAIL;

	// load sownsample shader
	downsamplePS = CreateSimplePixelShader();
	if (!downsamplePS->LoadShaderFile(L"./Assets/Shaders/DownsamplerPS.cso"))
		return E_FAIL;

	// load upsample shader
	upsamplePS = CreateSimplePixelShader();
	if (!upsamplePS->LoadShaderFile(L"./Assets/Shaders/UpsamplePS.cso"))
		return E_FAIL;

	// load horizontal shader
	horizontalBlurPS = CreateSimplePixelShader();
	if (!horizontalBlurPS->LoadShaderFile(L"./Assets/Shaders/HorizontalBlurPixelShader.cso"))
		return E_FAIL;

	// load vertical blur shader
	verticalBlurPS = CreateSimplePixelShader();
	if (!verticalBlurPS->LoadShaderFile(L"./Assets/Shaders/VerticalBlurPixelShader.cso"))
		return E_FAIL;

	// load vertical blur shader
	postPS = CreateSimplePixelShader();
	if (!postPS->LoadShaderFile(L"./Assets/Shaders/PostProcessPixelShader.cso"))
		return E_FAIL;

	// load def light vs
	deferredLightVS = CreateSimpleVertexShader();
	if (!deferredLightVS->LoadShaderFile(L"./Assets/Shaders/DeferredLightVS.cso"))
		return E_FAIL;

	// Pre post process ps
	prePostProcessPS = CreateSimplePixelShader();
	if (!prePostProcessPS->LoadShaderFile(L"./Assets/Shaders/PrePostProcessPS.cso"))
		return E_FAIL;

	// Other renderers
	particleRenderer = new ParticleRenderer(*this);
	hr = particleRenderer->Initialize();
	if (FAILED(hr))
		return hr;

	skyRenderer = new SkyRenderer(*this);
	hr = skyRenderer->init();
	if (FAILED(hr))
		return hr;

	lightRenderer = new LightRenderer(*this);
	hr = lightRenderer->Initialize(deferredVS, window->GetWidth(), window->GetHeight());
	if (FAILED(hr))
		return hr;
	
	DirectionalLight* test = lightRenderer->CreateDirectionalLight("testD", true);
	test->diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 0.7f, 1.0f);
	test->ambientColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	test->direction = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	test->intensity = 1.0f;

	/*
	test = lightRenderer->CreateDirectionalLight("testD2", true);
	test->diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	test->ambientColor = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	test->direction = DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f);
	test->intensity = 1.0f;
	*/


	PointLight* pltest = lightRenderer->CreatePointLight(PointLightAttenuation::ATTEN_FROM_RADIUS, "plTest", true);
	pltest->diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	pltest->position = DirectX::XMFLOAT3(0, 0, -1);
	pltest->atenConstant = 1.0f;
	pltest->atenLinear = 1.0f;
	pltest->atenQuadratic = 16.0f;
	pltest->radius = 16.0f;

	/*
	directionalLights[0].DiffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLights[0].Direction = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	directionalLights[0].Intensity = 1.0f;


	pointLights[0].DiffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pointLights[0].Position = DirectX::XMFLOAT3(0, 0, 0);
	pointLights[0].Intensity = 1.0f;
	*/

	// Return the "everything is ok" HRESULT value
	return S_OK;
}

// --------------------------------------------------------
// Clears all render targets
// --------------------------------------------------------
inline void Renderer::ClearRenderTargets()
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear all render targets
	for (size_t i = 0; i < BUFFER_COUNT; i++)
		context->ClearRenderTargetView(targetViews[i], color);

	for (size_t i = 0; i < 3; i++)
		context->ClearRenderTargetView(postProcessRTVs[i], color);

	for (size_t i = 0; i < 2; i++)
		context->ClearRenderTargetView(halfRTVs[i], color);

	lightRenderer->ClearRTV();

	// Clear depth buffer
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
}

// --------------------------------------------------------
// Renders the currently set UI panel. Using a single 
// spriteBatch.
// --------------------------------------------------------
inline void Renderer::RenderUI()
{
	assert(panel != nullptr);

	// Start sprite batch
	spriteBatch->Begin();

	// draw panel
	panel->Draw(spriteBatch, fontMap);

	// Stop sprite batch
	spriteBatch->End();
}

// --------------------------------------------------------
// Initialize the renderer by attaching it to a window.
// --------------------------------------------------------
Renderer* const Renderer::Initialize(DXWindow* const window)
{
	// Ensure not already initialized
	assert(instance == nullptr);

	// Initialize renderer
	instance = new Renderer(window);

	// return instance after init
	return instance;
}

// --------------------------------------------------------
// Get the current instance of this renderer
// --------------------------------------------------------
Renderer * const Renderer::Instance()
{
	// Ensure initialized
	assert(instance != nullptr);

	// Return our instance
	return instance;
}

// --------------------------------------------------------
// Shutdown the renderer instance
// --------------------------------------------------------
void Renderer::Shutdown()
{
	if (instance)
	{
		delete instance;
		instance = nullptr;
	}
}

// --------------------------------------------------------
// Stage an entity to be rendered.
// NOTE: Currently need to unstage and restage entity when
// material is modified/changed. This is already done in
// Entity->SetMaterial() for you.
//
// entity - the entity to render
// --------------------------------------------------------
void Renderer::StageEntity(Entity * const entity)
{
	// add to unordered multimap
	renderBatches.insert(std::make_pair(entity->GetMaterial()->GetID(), entity));
}


// --------------------------------------------------------
// Removes an entity from its render batch. This will cause
// it to not render.
// NOTE: Currently need to unstage and restage entity when
// material is modified/changed. This is already done in
// Entity->SetMaterial() for you.
//
// entity - the entity to remove from the render batch
// --------------------------------------------------------
void Renderer::UnstageEntity(Entity * const entity)
{
	// grab bin
	auto bucket = renderBatches.equal_range(entity->GetMaterial()->GetID());
	auto iterator = bucket.first;

	// loop until we find the entity and remove
	// WARNING: ENTITY POINTER SHOULD NEVER EVER CHANGE SINCE THE START OF
	// THE PROGRAM. DO NOT MOVE THE POINTER!
	for (; iterator != bucket.second; iterator++)
	{
		if (iterator->second == entity)
		{
			renderBatches.erase(iterator);
			break;
		}
	}
}

// --------------------------------------------------------
// Renders currently staged objects to a given camera
//
// camera - view point to use when rendering objects
// --------------------------------------------------------
void Renderer::Render(const Camera * const camera)
{
	// Shaders we will work with for each bucket
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Current vert buffer we're drawing
	const Mesh* currMesh;
	Material* currMaterial;
	Entity* currEntity;
	ID3D11Buffer* currVertBuff;
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Camera information that will not change mid-render
	XMFLOAT4X4 view = camera->GetViewMatrix();
	XMFLOAT4X4 projection = camera->GetProjectionMatrix();

	// Clear
	ClearRenderTargets();

	// Set render targets to textures
	// Our deferred renderer will now output to our render target textures
	context->OMSetRenderTargets(BUFFER_COUNT, targetViews, depthStencilView);

	// Iterate through each bucket
	for (auto it = renderBatches.begin(); it != renderBatches.end();)
	{
		auto bucket = renderBatches.equal_range(it->first);

		// Grab current shaders to work with
		currMaterial = bucket.first->second->GetMaterial();
		vertexShader = currMaterial->GetVertexShader();
		pixelShader = currMaterial->GetPixelShader();

		// -- Camera --
		vertexShader->SetMatrix4x4("view", view);
		vertexShader->SetMatrix4x4("projection", projection);

		// -- Set material specific information --
		currMaterial->PrepareMaterial();

		// Set stencil stuff
		context->OMSetDepthStencilState(depthStencilState, currMaterial->stencilID);

		// -- Copy pixel data --
		pixelShader->CopyAllBufferData();

		for (auto bucketIt = bucket.first; bucketIt != bucket.second; bucketIt++)
		{
			// How to pass in camera location for a blinn-phone material
			// when I can only supply the vertexShader and pixelShader?
			// The camera location is something that sits constant during these calcs
			// We need to make a function in material that prepares constant information
			// for the given shader?

			// -- Grab current entity --
			currEntity = bucketIt->second;

			// -- Set entity specific info --
			// below exist for every entity.
			vertexShader->SetMatrix4x4("world",
				currEntity->transform.GetWorldMatrix());
			vertexShader->SetMatrix4x4("inverseTransposeWorld",
				currEntity->transform.GetInverseTransposeWorldMatrix());

			// -- Copy vertex data --
			vertexShader->CopyAllBufferData();

			// -- Set shaders --
			vertexShader->SetShader();
			pixelShader->SetShader();

			// -- Draw model --
			// Set buffers in the input assembler
			//  - Do this ONCE PER OBJECT you're drawing, since each object might
			//    have different geometry.
			currMesh = bucketIt->second->GetMesh();
			currVertBuff = currMesh->GetVertexBuffer();
			context->IASetVertexBuffers(0, 1, &currVertBuff, &stride, &offset);
			context->IASetIndexBuffer(currMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

			// Finally do the actual drawing
			//  - Do this ONCE PER OBJECT you intend to draw
			//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
			//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
			//     vertices in the currently set VERTEX BUFFER
			context->DrawIndexed(
				currMesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
				0,     // Offset to the first index we want to use
				0);    // Offset to add to each index when looking up vertices
		}

		// move to next bucket
		it = bucket.second;
	}

	// -- Particles (deferred rendering) --
	particleRenderer->Render(camera);

	// Turn off ZBUFFER
	context->OMSetDepthStencilState(nullptr, 0);

	// -- Sky --
	skyRenderer->Render(camera);
	
	// Unbind shader srv and sampler state from last ps
	static ID3D11ShaderResourceView* const null[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	context->PSSetShaderResources(0, 5, null);

	// Sky
	//skyRenderer->Render(camera);
	
	// -- Lighting --
	context->OMSetBlendState(addBlendState, nullptr, 0xffffffff);
	lightRenderer->Render(camera);
	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	// -- Combine --
	context->OMSetRenderTargets(3, postProcessRTVs, nullptr);
	prePostProcessPS->SetShaderResourceView("colorTexture", targetSRVs[0]);
	prePostProcessPS->SetShaderResourceView("lightTexture", lightRenderer->lightSRV);
	prePostProcessPS->SetShaderResourceView("emissionTexture", targetSRVs[3]);
	prePostProcessPS->SetSamplerState("deferredSampler", targetSampler);
	prePostProcessPS->SetFloat("ColorThreshold", colorThreshold);
	prePostProcessPS->SetFloat("GlowPercentage", glowPercentage);
	prePostProcessPS->CopyAllBufferData();
	prePostProcessPS->SetShader();
	deferredVS->SetShader();
	context->Draw(3, 0);

	//Clear target views to reuse
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (size_t i = 0; i < BUFFER_COUNT; i++)
		context->ClearRenderTargetView(targetViews[i], color);
	
	// Post-processing
	//Create smaller textures - glow
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &halfRTVs[0], nullptr);
	downsamplePS->SetShaderResourceView("tex", postProcessSRVs[2]);//from deferredPS 0=lighting 1=bloom 2=glow
	downsamplePS->SetSamplerState("texSampler", targetSampler);
	downsamplePS->SetFloat("texelWidth", texelWidth);
	downsamplePS->SetFloat("texelHeight", texelHeight);
	downsamplePS->SetFloat("sizeMod", 2);
	// -- Copy pixel data --
	downsamplePS->CopyAllBufferData();
	// Set pixel data
	deferredVS->SetShader();
	downsamplePS->SetShader();
	context->Draw(3, 0);

	// Horizontal blur - bloom
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &targetViews[0], nullptr);//go elsewhere --> 0 in targetViews (recycling)
	horizontalBlurPS->SetShaderResourceView("blurTexture", postProcessSRVs[1]);//from deferredPS 0=lighting 1=pixels to blur
	horizontalBlurPS->SetSamplerState("blurSampler", targetSampler);
	horizontalBlurPS->SetFloat("blurDistance", blurDist);
	horizontalBlurPS->SetFloat("texelSize", texelWidth);
	// -- Copy pixel data --
	horizontalBlurPS->CopyAllBufferData();
	// Set pixel data
	horizontalBlurPS->SetShader();
	context->Draw(3, 0);

	// Horizontal blur - glow (full)
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &targetViews[2], nullptr);
	horizontalBlurPS->SetShaderResourceView("blurTexture", postProcessSRVs[2]);
	horizontalBlurPS->SetSamplerState("blurSampler", targetSampler);
	horizontalBlurPS->SetFloat("blurDistance", glowDist);
	horizontalBlurPS->SetFloat("texelSize", texelWidth);
	// -- Copy pixel data --
	horizontalBlurPS->CopyAllBufferData();
	// Set pixel data
	horizontalBlurPS->SetShader();
	context->Draw(3, 0);

	// Vertical blur - bloom
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &targetViews[1], nullptr);//go elsewhere --> 1 in targetViews (recycling)
	verticalBlurPS->SetShaderResourceView("horizBlurTexture", targetSRVs[0]);
	verticalBlurPS->SetSamplerState("blurSampler", targetSampler);
	verticalBlurPS->SetFloat("blurDistance", blurDist);
	verticalBlurPS->SetFloat("texelSize", texelHeight);
	// -- Copy pixel data --
	verticalBlurPS->CopyAllBufferData();
	// Set pixel data
	verticalBlurPS->SetShader();
	context->Draw(3, 0);

	// Vertical blur - glow (full)
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &targetViews[3], nullptr);
	verticalBlurPS->SetShaderResourceView("horizBlurTexture", targetSRVs[2]);
	verticalBlurPS->SetSamplerState("blurSampler", targetSampler);
	verticalBlurPS->SetFloat("blurDistance", glowDist);
	verticalBlurPS->SetFloat("texelSize", texelHeight);
	// -- Copy pixel data --
	verticalBlurPS->CopyAllBufferData();
	// Set pixel data
	verticalBlurPS->SetShader();
	context->Draw(3, 0);

	context->RSSetViewports(1, &halfViewport);
	// Horizontal blur - glow (half)
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &halfRTVs[1], nullptr);
	horizontalBlurPS->SetShaderResourceView("blurTexture", halfSRVs[0]);
	horizontalBlurPS->SetSamplerState("blurSampler", targetSampler);
	horizontalBlurPS->SetFloat("blurDistance", glowDist);
	horizontalBlurPS->SetFloat("texelSize", texelWidth * 2);
	// -- Copy pixel data --
	horizontalBlurPS->CopyAllBufferData();
	// Set pixel data
	horizontalBlurPS->SetShader();
	context->Draw(3, 0);

	// Vertical blur - glow (half)
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &halfRTVs[0], nullptr);
	verticalBlurPS->SetShaderResourceView("horizBlurTexture", halfSRVs[1]);
	verticalBlurPS->SetSamplerState("blurSampler", targetSampler);
	verticalBlurPS->SetFloat("blurDistance", glowDist);
	verticalBlurPS->SetFloat("texelSize", texelHeight * 2);
	// -- Copy pixel data --
	verticalBlurPS->CopyAllBufferData();
	// Set pixel data
	verticalBlurPS->SetShader();
	context->Draw(3, 0);
	
	context->RSSetViewports(1, &viewport);
	//Recombine smaller textures
	context->ClearRenderTargetView(targetViews[2], color);//might not be necessary
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &targetViews[2], nullptr);
	upsamplePS->SetShaderResourceView("tex0", targetSRVs[3]);//full glow
	upsamplePS->SetShaderResourceView("tex1", halfSRVs[0]);//half glow
	// -- Copy pixel data --
	upsamplePS->CopyAllBufferData();
	// Set pixel data
	upsamplePS->SetShader();
	context->Draw(3, 0);
	
	// volumetric lighting
	context->PSSetShaderResources(0, 5, null);
	context->OMSetRenderTargets(1, &targetViews[3], nullptr);
	//-2,1,130
	//.1,.1 is approximately position of sun, if light will move later can pass in directionalLights[0].direction mapped to value between 0 and 1(for screen space)
	XMFLOAT2 ScreenLightPos = XMFLOAT2(200.0f, 200.0f);
	float Exposure = .03f;//.05 is less in your face
	float Decay = .99f;//0-1//apparently don't change this, really messes with it, makes it look a lot worse
	float Density = 0.5f;//higher looks worse, lower makes rays too short
	float Weight = .09f;//.2 is suggested, can vary
	int NumSamples = 100;//200 //slightly better looking ~30 fps drop
	volumetricLightingPS->SetShaderResourceView("volumetricTexture", depthSRV);
	volumetricLightingPS->SetSamplerState("volumetricSampler", targetSampler);
	volumetricLightingPS->SetFloat2("ScreenLightPos", ScreenLightPos);
	volumetricLightingPS->SetFloat("Exposure", Exposure);
	volumetricLightingPS->SetFloat("Decay", Decay);
	volumetricLightingPS->SetFloat("Density", Density);
	volumetricLightingPS->SetFloat("Weight", Weight);
	volumetricLightingPS->SetInt("NumSamples", NumSamples);

	// -- Copy pixel data --
	volumetricLightingPS->CopyAllBufferData();

	// Set pixel data
	volumetricLightingPS->SetShader();

	context->Draw(3, 0);
	context->PSSetShaderResources(0, 5, null);
	//Add all post processing effects together

	context->OMSetRenderTargets(1, &backBufferRTV, nullptr);
	
	postPS->SetShaderResourceView("colorTexture", postProcessSRVs[0]);
	postPS->SetShaderResourceView("bloomTexture", targetSRVs[1]);
	postPS->SetShaderResourceView("glowTexture", targetSRVs[2]);
	postPS->SetShaderResourceView("volumetricTexture", targetSRVs[3]);
	postPS->SetSamplerState("finalSampler", targetSampler);

	postPS->CopyAllBufferData();
	postPS->SetShader();

	context->Draw(3, 0);

	// Render UI
	if (panel)
		RenderUI();

	// Unbind all sampler states and srvs
	context->PSSetShaderResources(0, 5, null);

	// Fixes depth buffer issue regarding SpriteBatch2D
	context->OMSetDepthStencilState(nullptr, 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

// --------------------------------------------------------
// Update exclusively for compute shaders that should be 
// dispatched during the update section of the engine.
//
// dt - delta time of last two frames
// totalTime - total time application has been open
// --------------------------------------------------------
void Renderer::UpdateCS(float dt, float totalTime)
{
	particleRenderer->Update(dt, totalTime);
}

// --------------------------------------------------------
// When the window is resized, the underlying 
// buffers (textures) must also be resized to match.
//
// If we don't do this, the window size and our rendering
// resolution won't match up.  This can result in odd
// stretching/skewing.
//
// width - new width to resize to
// height - new height to resize to
// --------------------------------------------------------
void Renderer::OnResize(unsigned int width, unsigned int height)
{
	//used in post processing
	texelWidth = 1.0f / width;
	texelHeight = 1.0f / height;
	/*
	// Release existing DirectX views and buffers
	if (depthStencilView) { depthStencilView->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }

	// Resize the underlying swap chain buffers
	swapChain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0);

	// Recreate the render target view for the back buffer
	// texture, then release our local texture reference
	ID3D11Texture2D* backBufferTexture;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture));
	device->CreateRenderTargetView(backBufferTexture, 0, &backBufferRTV);
	backBufferTexture->Release();

	// Set up the description of the texture to use for the depth buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	// Create the depth buffer and its view, then
	// release our reference to the texture
	ID3D11Texture2D* depthBufferTexture;
	device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
	device->CreateDepthStencilView(depthBufferTexture, 0, &depthStencilView);
	depthBufferTexture->Release();

	// Bind the views to the pipeline, so rendering properly
	// uses their underlying textures
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	// Lastly, set up a viewport so we render into
	// to correct portion of the window
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
	*/
}

// --------------------------------------------------------
// Sets a single UI panel to be rendered next frame. You
// are expected to encapsulate multiple UI panels within a
// single UI panel since the UI panel is really just a
// container.
//
// panel - The panel to draw next frame
// --------------------------------------------------------
void Renderer::SetCurrentPanel(UIPanel * panel)
{
	this->panel = panel;
}

// --------------------------------------------------------
// Load a font into the font map to be used by panels
//
// name		- name of font, this will be used when grabbing
//			  the font from the font map.
// path		- path to the spritefont file to load
// --------------------------------------------------------
void Renderer::LoadFont(const char * const name, const wchar_t * const path)
{
	// Load font at name if it doesnt already exist
	if (!fontMap.count(name))
		fontMap[name] = new SpriteFont(device, path);
	else
		fprintf(stderr, "[UIRenderer] %s already loaded!\n", name);
}

// --------------------------------------------------------
// Create and return a simple vertex shader.
// --------------------------------------------------------
SimpleVertexShader * const Renderer::CreateSimpleVertexShader() const
{
	return new SimpleVertexShader(device, context);
}

// --------------------------------------------------------
// Create and return a simple pixel shader.
// --------------------------------------------------------
SimplePixelShader * const Renderer::CreateSimplePixelShader() const
{
	return new SimplePixelShader(device, context);
}

// --------------------------------------------------------
// Create and return a simple compute shader
// --------------------------------------------------------
SimpleComputeShader * const Renderer::CreateSimpleComputeShader() const
{
	return new SimpleComputeShader(device, context);
}

// --------------------------------------------------------
// Create and return a continuous particle emitter
// --------------------------------------------------------
ParticleEmitter * const Renderer::CreateContinuousParticleEmitter(std::string name, unsigned int particlesPerSeconds, float seconds) const
{
	return particleRenderer->CreateContinuousParticleEmitter(name, particlesPerSeconds, seconds);
}

// --------------------------------------------------------
// Create and return a burst particle emitter
// --------------------------------------------------------
ParticleEmitter * const Renderer::CreateBurstParticleEmitter(std::string name, unsigned int numParticles) const
{
	return particleRenderer->CreateBurstParticleEmitter(name, numParticles);
}

// --------------------------------------------------------
// Create and return a mesh.
// --------------------------------------------------------
Mesh * const Renderer::CreateMesh(const char * path) const
{
	if (!path)
		return nullptr;
	return new Mesh(path, device);
}

// --------------------------------------------------------
// Create and return a texture.
// TODO: Allow for different sampler options?
// --------------------------------------------------------
Texture2D * const Renderer::CreateTexture2D(const wchar_t * path, Texture2DType type, Texture2DFileType fileType)
{
	if (!path)
		return nullptr;
	return new Texture2D(path, type, fileType, objectTextureSampler, device, context);
}

// --------------------------------------------------------
// Gets the map of material IDs to entity pointers that are to be rendered.
// --------------------------------------------------------
const std::unordered_multimap<unsigned int, Entity*>& Renderer::GetRenderBatches() const
{
	return renderBatches;
}

// --------------------------------------------------------
// Removes all emitters from particle renderer
// --------------------------------------------------------
void Renderer::ReleaseParticleRenderer()
{
	particleRenderer->Release();
}

