#pragma once
#include "Entity.h"

// Entity that is fired by the player and does damage to enemies
class EntityProjectile :
	public virtual Entity
{
public:
	EntityProjectile(EntityFactory* entityFactory, std::string name, Mesh* mesh, Material* material);
	~EntityProjectile();

	void Update(float deltaTime, float totalTime) override;

	void Fire(XMFLOAT3 position, XMFLOAT3 direction, float speed);

	// Remove the bullet from the field (Moving it out of frame)
	void Remove();

	// Getters and Setters
	void SetSpeed(float speed);
	float GetSpeed();
	void SetDirection(XMFLOAT3 direction);
	XMFLOAT3* GetDirection();

protected:
	float speed;	// Speed the projectile moves at
	XMFLOAT3 direction;	// Direction the projectile moves towards

	ParticleEmitter* peTrail;	// Particle effect for trail

	void OnCollision(Collision collison) override;
};

