#pragma once
#include <unordered_map>
#include "UIPanel.h"
#include "StateManager.h"
#define MAX_TIMER_BUFFER_LEN 9 // 00:00.00\0

// UI for the actual game
class UIPanelGame : public UIPanel
{
public:
	UIPanelGame(float x, float y);
	virtual ~UIPanelGame();

	// Implement required functions
	// Draw UI
	void Draw(SpriteBatch* const spriteBatch,
		const std::unordered_map<const char*, SpriteFont*>& fontMap);
	void Update(float deltaTime, float totalTime);
	//void UpdateText(wstring _text);

	// Update the player's health
	void UpdateHealth(int health);

	// Set the time displayed in UI
	void SetGameTime(float gameTime);
private:
	float gameTime = 0;	// Time the player is alive
	int health = 0;	// The players health
	float x;	// xPos of UI
	float y;	// yPos of UI

	// Represents the healthbar
	struct HealthBar {
		SimpleMath::Rectangle rectangle;	// Represent position and scale.
		float maxWidth = 0;
		Texture2D* texture = nullptr;	// Texture of Button
	};
	HealthBar healthBar;
};