#pragma once
#include "Entity.h"
#include "EntityManagerProjectile.h"
#include "EntityEnemy.h"

#define ARENA_TOP_WALL 2.0f
#define ARENA_BOTTOM_WALL -ARENA_TOP_WALL
#define ARENA_RIGHT_WALL ARENA_TOP_WALL * 1.77777777778f
#define ARENA_LEFT_WALL -ARENA_RIGHT_WALL

// Entity controlled by the player.
class EntityPlayer :
	public Entity
{
public:
	EntityPlayer(EntityFactory* entityFactory, std::string name, Mesh* mesh, Material* material);
	~EntityPlayer();

	// Set new update
	void Update(float deltaTime, float totalTime) override;
	
	// Setters and Getters
	void SetSpeed(float speed);
	float GetSpeed();
	void SetProjectileManager(EntityManagerProjectile* projectileManager);

	// Change health by amount
	void ChangeHealth(int healthDelta);

	// Handles mouse press for mouse firing controls
	void OnMousePressed(float x, float y);

	int health, maxHealth;

protected:
	float speed;		// The speed the player can move at
	float fireTimer;	// Stores time since last firing
	float fireRate;		// The maximum rate of firing
	EntityManagerProjectile* projectileManager;	// Pointer to manager

	// Particle Systems
	ParticleEmitter* peEngineExhaust;
	ParticleEmitter* peFireProjectile;
	ParticleEmitter* peExplosionDebris;
	ParticleEmitter* peExplosionFireball;

	void FireProjectile(XMFLOAT3 direction);	// Fire a projectile

	void OnCollision(Collision collision) override;	
};

