#pragma once
#include "Entity.h"
#include "MaterialEnemy.h"

// Enemy entity
// Goes straight towards the player.
// Does damage when it hits the player
// Shrinks when damaged, grows when it heals
// Heals gradually overtime
class EntityEnemy :
	public Entity
{
public:
	EntityEnemy(EntityFactory* entityFactory, std::string name, Mesh* mesh, Material* material);
	~EntityEnemy();

	void Update(float deltaTime, float totalTime) override;

	// Move to random position.
	void MoveToRandomPosition();

	// Change health by amount
	void ChangeHealth(float healthDelta);

	// Setters and Getters
	void SetSpeed(float speed);
	float GetSpeed();
	void SetHealth(float health);
	float GetHealth();
	void SetMaxHealth(float healthMax);
	float GetMaxHealth();
	void SetTarget(Entity* target);
	Entity* GetTarget();
	void SetDirection(XMFLOAT3 direction);
	const XMFLOAT3* const GetDirection() const;

protected:
	float maxScale;		// Max scale of enemy
	float speed;		// Speed the enemy moves at
	float health;		// The enemies current health.
	float healthMax;	// The max health of the enemy

	Entity* target;		// The target of the enemy
	XMFLOAT3 direction;	// Direction the enemy is looking at
	XMFLOAT3 rotationAxis;	// Axis the enemy rotates around

	// Material
	MaterialEnemy* enemyMaterial;

	// Particle Systems
	ParticleEmitter* peExplosionDebris;
	ParticleEmitter* peExplosionFireball;


	// Implements unique collision behavior
	void OnCollision(Collision collision) override;
};

