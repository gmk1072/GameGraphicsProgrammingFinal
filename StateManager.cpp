#include "StateManager.h"
#include "MemoryDebug.h"

StateManager::StateManager()
{
}

StateManager::~StateManager()
{
	for (auto iter = scenesMap.begin(); iter != scenesMap.end(); ++iter)
	{
		delete iter->second;
	}
}

GameState& StateManager::GetCurrentState()
{
	return currentState;
}

void StateManager::AddScene(GameState state, Scene * scene)
{
	if (scenesMap[state] == nullptr)
	{
		scenesMap[state] = scene;
	}
}

Scene* StateManager::GetCurrentScene()
{
	return currentScene;
}

void StateManager::SetEntityFactory(EntityFactory* entityFactory)
{
	this->entityFactory = entityFactory;
}

void StateManager::SetMeshes(std::unordered_map<const char*, Mesh*>* meshes)
{
	this->meshes = meshes;
}

void StateManager::SetMaterials(std::unordered_map<const char*, Material*>* materials)
{
	this->materials = materials;
}


void StateManager::SetState(GameState newState)
{
	UnloadScene(currentScene);
	currentScene = scenesMap[newState];
	LoadScene(currentScene);
}

void StateManager::LoadScene(Scene* scene)
{
	scene->CreateSceneEntities(*entityFactory, *meshes, *materials);
	//set ui renderer junkety stuff 
	Renderer::Instance()->SetCurrentPanel(currentScene->GetUIPanel());
}

void StateManager::UnloadScene(Scene* scene)
{
	entityFactory->Release();
	Renderer::Instance()->ReleaseParticleRenderer();
}
