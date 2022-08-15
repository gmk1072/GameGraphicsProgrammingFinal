#include "Scene.h"
#include "MemoryDebug.h"

Scene::Scene(Game* game) :
	game(game)
{
}

Scene::~Scene()
{
	// Delete UI object
	if (uiPanel)
		delete uiPanel;
}

void Scene::UpdateScene(float deltaTime, float totalTime)
{
}

void Scene::OnMousePressed(float x, float y)
{
}

UIPanel* Scene::GetUIPanel()
{
	return this->uiPanel;
}
