#pragma once
#include "UIPanel.h"
#include "StateManager.h"

class Game;

// UI for the menu of the game
class UIPanelMenu :
	public UIPanel
{
public:
	UIPanelMenu(Game* game, StateManager* stateManager);
	~UIPanelMenu();

	// Draw menu
	void Draw(SpriteBatch* const spriteBatch,
		const std::unordered_map<const char*, SpriteFont*>& fontMap) override;

	// Handle mouse press on menu
	void MousePressed(float x, float y);

private:	
	Game* game;	// Pointer to game
	StateManager* stateManager;	// Pointer to state manager

	// Represents a UI element that can be pressed.
	struct Button {
		SimpleMath::Rectangle rectangle;	// Represent position and scale.
		wstring text;	// Text of Button
		Texture2D* texture = nullptr;	// Texture of Button
		void(UIPanelMenu::*onPressed) (void) = nullptr;	// Function called when pressed

		// Check if it is pressed.
		bool CheckIsPressed(float x, float y) {
			return	rectangle.x <= x && 
				x <= rectangle.x + rectangle.width &&
				rectangle.y <= y &&
				y <= rectangle.y + rectangle.height;
		}
	};

	// Buttons in UI
	std::unordered_map<const char*, Button> buttons;

	void PlayGame();	// Set game state to game
	void ExitGame();	// Exits the game
};

