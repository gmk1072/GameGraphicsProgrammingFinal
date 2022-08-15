#include "UIPanelMenu.h"
#include "Game.h"

UIPanelMenu::UIPanelMenu(Game* game, StateManager* stateManager) :
	game(game),
	stateManager(stateManager)
{
	buttons = std::unordered_map<const char*, Button>();

	// Create "Logo" button (Does nothing when pressed
	Button logo;
	logo.rectangle = SimpleMath::Rectangle(448, 150, 364, 91);
	logo.texture = Renderer::Instance()->CreateTexture2D(L"./Assets/Textures/ui_logo.tif", Texture2DType::ALBEDO);
	logo.text = L"";
	buttons["logo"] = logo;

	// Create "Start Game" button
	Button startGame;
	startGame.rectangle = SimpleMath::Rectangle(544, 300, 192, 48);
	startGame.texture = Renderer::Instance()->CreateTexture2D(L"./Assets/Textures/ui_button.tif", Texture2DType::ALBEDO);
	startGame.text = L"Play Game";
	startGame.onPressed = &UIPanelMenu::PlayGame;
	buttons["startGame"] = startGame;

	// Create "Exit Game" button
	Button exitGame;
	exitGame.rectangle = SimpleMath::Rectangle(544, 348, 192, 48);
	exitGame.texture = Renderer::Instance()->CreateTexture2D(L"./Assets/Textures/ui_button.tif", Texture2DType::ALBEDO);
	exitGame.text = L"Exit Game";
	exitGame.onPressed = &UIPanelMenu::ExitGame;
	buttons["exitGame"] = exitGame;
}

UIPanelMenu::~UIPanelMenu()
{
	// Delete the textures used by the buttons
	for (auto iter = buttons.begin(); iter != buttons.end(); ++iter)
	{
		if (iter->second.texture)
			delete iter->second.texture;
	}
}

void UIPanelMenu::Draw(SpriteBatch * const spriteBatch, const std::unordered_map<const char*, SpriteFont*>& fontMap)
{
	// Draw button
	for (auto iter = buttons.begin(); iter != buttons.end(); ++iter)
	{
		Button button = iter->second;
		spriteBatch->Draw(button.texture->GetSRV(), button.rectangle);
		fontMap.at("arial")->DrawString(spriteBatch, button.text.c_str(), XMFLOAT2(button.rectangle.x, button.rectangle.y));
	}
}

void UIPanelMenu::MousePressed(float x, float y)
{
	// Go through each button to see if the mouse is pressed.
	for (auto iter = buttons.begin(); iter != buttons.end(); ++iter)
	{
		Button button = iter->second;

		// If the button is pressed, call function pointer
		if (button.CheckIsPressed(x, y) && button.onPressed != nullptr)
			(this->*button.onPressed)();
	}
}

void UIPanelMenu::PlayGame()
{
	stateManager->SetState(GameState::GAME);
}

void UIPanelMenu::ExitGame()
{
	game->Quit();
}
