#pragma once
#include "Entity.h"
#include "EntityProjectile.h"

// Maintains a list of projectile entities that can be fired by the player.
// Means we do not have to create a new projectile each time.
class EntityManagerProjectile :
	public Entity
{
public:
	EntityManagerProjectile(EntityFactory* entityFactory, std::string name);
	~EntityManagerProjectile();

	virtual void Update(float deltaTime, float totalTime) override;	// Overrided but not used

	void SetProjectiles(std::vector<EntityProjectile*> projectiles);	// Set the entities being used
	EntityProjectile* GetProjectile();	// Get a projectile to be used by the player

protected:
	unsigned int numberOfProjectiles;	// Number of projectiles in manager
	unsigned int currentIndex;	// The index of projectile to be returned in the next GetProjectile() call.
	std::vector<EntityProjectile*> projectiles;	// List of projectiles
};

