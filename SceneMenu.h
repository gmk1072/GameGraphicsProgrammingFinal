#pragma once
#include "Scene.h"
#include "StateManager.h"
#include "UIPanelMenu.h"

class StateManager;

// Scene used for the opening menu of the game
class SceneMenu :
	public Scene
{
public:
	SceneMenu(Game* game, StateManager& stateManager);
	virtual ~SceneMenu();

	// Create background cube entities
	void CreateSceneEntities(EntityFactory& entityFactory, std::unordered_map<const char*, Mesh*>& meshes, std::unordered_map<const char*, Material*>& materials) override;

	// Handle mouse press
	void OnMousePressed(float x, float y) override;
private:
	// User interface for menu
	UIPanelMenu* uiPanelMenu;
};

