#include "UIPanelGame.h"
#include "MemoryDebug.h"

UIPanelGame::UIPanelGame(float x, float y) :
	x(x),
	y(y)
{
	// Sets healthbar properties
	healthBar.texture = Renderer::Instance()->CreateTexture2D(L"./Assets/Textures/ui_healthbar.tif", Texture2DType::ALBEDO);
	healthBar.rectangle = SimpleMath::Rectangle(x + 8, y + 32, 180, 32);
	healthBar.maxWidth = 180;
}

UIPanelGame::~UIPanelGame()
{
	delete healthBar.texture;
}

void UIPanelGame::Draw(SpriteBatch * const spriteBatch,
	const std::unordered_map<const char*, SpriteFont*>& fontMap)
{
	// Print time and other game related UI stuff
	auto timerString = std::to_wstring((int)(gameTime));
	fontMap.at("arial")->DrawString(spriteBatch, timerString.c_str(), XMFLOAT2(x + 8, y));
	spriteBatch->Draw(healthBar.texture->GetSRV(), healthBar.rectangle);	// Draw healthbar

	// On death, show option
	if (health <= 0) {
		fontMap.at("arial")->DrawString(spriteBatch, L"Press '4': restart", XMFLOAT2(x + 8, y + 32));
		fontMap.at("arial")->DrawString(spriteBatch, L"Press '3': menu", XMFLOAT2(x + 8, y + 64));
	}
}

void UIPanelGame::Update(float deltaTime, float totalTime)
{
}

void UIPanelGame::UpdateHealth(int health)
{
	// On new health, update health bar
	this->health = health;
	healthBar.rectangle.width = health / 100.0f * healthBar.maxWidth;
}

void UIPanelGame::SetGameTime(float gameTime)
{
	this->gameTime = gameTime;
}
