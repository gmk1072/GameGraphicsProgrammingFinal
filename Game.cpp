#include "Game.h"
#include "MemoryDebug.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXWindow(
		hInstance,		   // The application's handle
		"&Poly_Star*",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	pixelShader_normal = 0;
	vertexShader_parallax = 0;
	pixelShader_parallax = 0;
	renderer = nullptr;
	collisionManager = nullptr;
	stateManager = StateManager();

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Free all entities
	entityFactory.Release();

	// Free all meshes
	for (auto it = meshes.begin(); it != meshes.end(); it++)
		delete it->second;

	// Free all textures
	for (auto it = textures.begin(); it != textures.end(); it++)
		delete it->second;

	// Free all materials
	for (auto it = materials.begin(); it != materials.end(); it++)
		delete it->second;

	// Clean up cameras
	delete debugCamera;
	delete gameCamera;

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete pixelShader_normal;
	delete vertexShader_enemy;
	delete pixelShader_parallax;
	delete vertexShader_parallax;

	// Shutdown renderer
	Renderer::Shutdown();

	// Shutdown Managers
	CollisionManager::Shutdown();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Initialize renderer singleton/DX
	renderer = Renderer::Initialize(this);
	collisionManager = CollisionManager::Initialize(0.25f, XMFLOAT3(3, 3, 0.5));


	// Setup Scenes and State Manager
	stateManager.SetEntityFactory(&entityFactory);
	stateManager.SetMaterials(&materials);
	stateManager.SetMeshes(&meshes);
	stateManager.AddScene(GameState::GAME, new SceneGame(this));
	stateManager.AddScene(GameState::MAIN_MENU, new SceneMenu(this, stateManager));

	// Initialize starting mouse location to center of screen
	prevMousePos.x = GetWidth() / 2;
	prevMousePos.y = GetHeight() / 2;

	// Create Cameras
	CreateCameras();

	// Load Assets
	LoadShaders();
	CreateBasicGeometry();

	// Load font 
	renderer->LoadFont("arial", L"./Assets/Font/Arial.spritefont");

	// Load the first scene
	LoadDefaultScene();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = renderer->CreateSimpleVertexShader();
	if (!vertexShader->LoadShaderFile(L"./Assets/Shaders/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = renderer->CreateSimplePixelShader();
	if (!pixelShader->LoadShaderFile(L"./Assets/Shaders/DefferedPixelShader.cso"))
		pixelShader->LoadShaderFile(L"DefferedPixelShader.cso");

	pixelShader_normal = renderer->CreateSimplePixelShader();
	if (!pixelShader_normal->LoadShaderFile(L"./Assets/Shaders/DefferedPixelShader_NormalMap.cso"))
		pixelShader_normal->LoadShaderFile(L"DefferedPixelShader_NormalMap.cso");

	vertexShader_enemy = renderer->CreateSimpleVertexShader();
	if (!vertexShader_enemy->LoadShaderFile(L"./Assets/Shaders/EnemyVS.cso"))
		vertexShader_enemy->LoadShaderFile(L"EnemyVS.cso");

	pixelShader_parallax = renderer->CreateSimplePixelShader();
	if (!pixelShader_parallax->LoadShaderFile(L"./Assets/Shaders/ParallaxPS.cso"))
		pixelShader_parallax->LoadShaderFile(L"ParallaxPS.cso");

	vertexShader_parallax = renderer->CreateSimpleVertexShader();
	if (!vertexShader_parallax->LoadShaderFile(L"./Assets/Shaders/ParallaxVS.cso"))
		vertexShader_parallax->LoadShaderFile(L"ParallaxVS.cso");

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D cameras
// --------------------------------------------------------
void Game::CreateCameras()
{
	// Create debug camera
	debugCamera = new CameraDebug();
	debugCamera->transform.Move(0, 0, -3.0f);

	// Create game camera
	gameCamera = new CameraGame();
	gameCamera->transform.Move(0, 0, -100.0f);
	gameCamera->SetViewHeight(GAME_HEIGHT);

	// Set game camera to active
	activeCamera = gameCamera;

	// Setup projection matrix
	debugCamera->UpdateProjectionMatrix((float)GetWidth() / GetHeight());
	gameCamera->UpdateProjectionMatrix((float)GetWidth() / GetHeight());
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Texture 1 from http://www.textures.com/download/3dscans0029/126909
	// Texture 2 from http://www.textures.com/download/3dscans0014/126018
	// Load up textures
	textures["brick"] = renderer->CreateTexture2D(L"./Assets/Textures/TexturesCom_BrownBricks_albedo_M.tif", Texture2DType::ALBEDO);
	textures["sand"] = renderer->CreateTexture2D(L"./Assets/Textures/TexturesCom_DesertSand1_albedo_M.tif", Texture2DType::ALBEDO);
	textures["stone"] = renderer->CreateTexture2D(L"./Assets/Textures/TexturesCom_StoneSurface_albedo_M.tif", Texture2DType::ALBEDO);
	textures["brick_norm"] = renderer->CreateTexture2D(L"./Assets/Textures/TexturesCom_BrownBricks_normalmap_M.tif", Texture2DType::NORMAL);
	textures["no_emission"] = renderer->CreateTexture2D(L"", Texture2DType::EMISSION);
	textures["sun"] = renderer->CreateTexture2D(L"./Assets/Textures/sun_Emission.tif", Texture2DType::ALBEDO);
	textures["sun_emission"] = renderer->CreateTexture2D(L"./Assets/Textures/sun_Emission.tif", Texture2DType::EMISSION);
	textures["enemy_albedo"] = renderer->CreateTexture2D(L"./Assets/Textures/enemy_Albedo.tif", Texture2DType::ALBEDO);
	textures["enemy_normal"] = renderer->CreateTexture2D(L"./Assets/Textures/enemy_Normal.tif", Texture2DType::NORMAL);
	textures["enemy_emission"] = renderer->CreateTexture2D(L"./Assets/Textures/enemy_Emission.tif", Texture2DType::EMISSION);
	textures["brick_depth"] = renderer->CreateTexture2D(L"./Assets/Textures/brick_depth.tif", Texture2DType::PARALLAX);

	// Create our materials
	materials["brick"] = new MaterialParallax(vertexShader_parallax, pixelShader_parallax, textures["brick"], textures["brick_norm"], textures["brick_depth"], activeCamera);
	materials["sand"] = new Material(vertexShader, pixelShader, textures["sand"]);
	materials["stone"] = new Material(vertexShader, pixelShader, textures["stone"]);
	materials["sun"] = new Material(vertexShader, pixelShader_normal, textures["sun"], textures["brick_norm"], textures["sun_emission"]);
	materials["enemy"] = new MaterialEnemy(vertexShader_enemy, pixelShader_normal, textures["enemy_albedo"], textures["enemy_normal"], textures["enemy_emission"]);

	// Load up all our meshes to a mesh dict
	meshes["cube"] = renderer->CreateMesh("./Assets/Models/cube.obj");
	meshes["helix"] = renderer->CreateMesh("./Assets/Models/helix.obj");
	meshes["torus"] = renderer->CreateMesh("./Assets/Models/torus.obj");
	meshes["cone"] = renderer->CreateMesh("./Assets/Models/cone.obj");
	meshes["cylinder"] = renderer->CreateMesh("./Assets/Models/cylinder.obj");
	meshes["sphere"] = renderer->CreateMesh("./Assets/Models/sphere.obj");
	meshes["enemy"] = renderer->CreateMesh("./Assets/Models/enemyBall.fbx");
	meshes["player"] = renderer->CreateMesh("./Assets/Models/playerShip.fbx");
	meshes["frame"] = renderer->CreateMesh("./Assets/Models/frame.fbx");
	meshes["planet"] = renderer->CreateMesh("./Assets/Models/planet.fbx");
	meshes["meteors"] = renderer->CreateMesh("./Assets/Models/meteors.fbx");
	meshes["sun"] = renderer->CreateMesh("./Assets/Models/sun.fbx");
}

// --------------------------------------------------------
// Creates entities using our generated meshes
// --------------------------------------------------------
void Game::LoadDefaultScene()
{
	stateManager.SetState(GameState::MAIN_MENU);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize(unsigned int width, unsigned int height)
{
	// Quick temp fix for checking if game is initialized
	if (renderer)
	{
		// Resize renderer outputs
		renderer->OnResize(width, height);

		// Update aspect ratio in cameras
		debugCamera->UpdateProjectionMatrix((float)width / height);
		gameCamera->UpdateProjectionMatrix((float)width / height);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	if (GetAsyncKeyState('1') & 0x8000)
	{
		activeCamera = gameCamera;
	}
	if (GetAsyncKeyState('2') & 0x8000)
	{
		activeCamera = debugCamera;
	}

	if (GetAsyncKeyState('3') & 0x8000)
	{
		stateManager.SetState(GameState::MAIN_MENU);
	}
	if (GetAsyncKeyState('4') & 0x8000)
	{
		stateManager.SetState(GameState::GAME);
	}

	//mouse pos
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	mouseX = static_cast<float>(cursorPos.x);
	mouseY = static_cast<float>(cursorPos.y);


	// Update camera
	activeCamera->Update(deltaTime, totalTime);

	// Update all entities
	entityFactory.UpdateEntities(deltaTime, totalTime);
	
	// set cursor to center of screen
	if(activeCamera == debugCamera)
		SetCursorPos(
			GetWindowLocation().x + GetWidth() / 2,
			GetWindowLocation().y + GetHeight() / 2
		);


	//check for collisions
	collisionManager->CollisionUpdate();

	// Update Scene
	if(stateManager.GetCurrentScene() != nullptr)
		stateManager.GetCurrentScene()->UpdateScene(deltaTime, totalTime);

	// Dispatch compute shaders
	renderer->UpdateCS(deltaTime, totalTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Render to active camera
	renderer->Render(activeCamera);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Send mouse press into the scene.
	stateManager.GetCurrentScene()->OnMousePressed(x, y);

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Capture the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// If the debug camera is active.
	if (activeCamera == debugCamera)
	{
		// left/right = rotate on X about Y
		// up/down = rotate on Y about Z
		debugCamera->RotateBy(static_cast<float>(x - (GetWidth() / 2.0f)) / 1000.0f,
			static_cast<float>(y - (GetHeight() / 2.0f)) / 1000.0f);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion