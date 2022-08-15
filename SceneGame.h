#pragma once
#include "Scene.h"
#include "UIPanelGame.h"
#include "ParticleEmitter.h"

// Represent the scene with the actual game
class SceneGame :
	public Scene
{
public:
	SceneGame(Game* game);
	virtual ~SceneGame();
	// Create entities used in game
	void CreateSceneEntities(EntityFactory& entityFactory, std::unordered_map<const char*, Mesh*>& meshes, std::unordered_map<const char*, Material*>& materials) override;
	// Update the state of the game
	void UpdateScene(float deltaTime, float totalTime) override;

	void OnMousePressed(float x, float y) override;

private:
	// Player
	EntityPlayer* player;

	//meteors
	Entity* meteors2;
	Entity* meteors;

	// User Interface
	UIPanelGame* uiGamePanel;

	// timer for ui
	float gameTime;
	int health;
};

