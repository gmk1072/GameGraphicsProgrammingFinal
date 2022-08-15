#pragma once
#include <vector>
#include "EntityFactory.h"
#include "Renderer.h"
#include "UIPanel.h"

class Game;

// Represents a scene the player can be in
class Scene
{
public:
	Scene(Game* game);
	virtual ~Scene();

	// Create the entities used in the scene
	virtual void CreateSceneEntities(EntityFactory& entityFactory, std::unordered_map<const char*, Mesh*>& meshes, std::unordered_map<const char*, Material*>& materials) = 0;

	virtual void UpdateScene(float deltaTime, float totalTime);	// Update the scene
	virtual void OnMousePressed(float x, float y);	// Handle a mouse press.

	UIPanel* GetUIPanel();	// Get the UI object used

protected:
	Game* game;	// Pointer to game object
	std::vector<Entity*> entityList;	// Vector of entities that this scene will load in when activated
	UIPanel* uiPanel;	// The UI for this scene.
};

