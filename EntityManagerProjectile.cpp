#include "EntityManagerProjectile.h"
#include "MemoryDebug.h"

EntityManagerProjectile::EntityManagerProjectile(EntityFactory* entityFactory, std::string name) :
	Entity(entityFactory, name)
{
	currentIndex = 0;	// Sets index of first available projectile
	SetIsUpdating(false);	// The Projectile Manager does not update every day.
}


EntityManagerProjectile::~EntityManagerProjectile()
{
}

void EntityManagerProjectile::SetProjectiles(std::vector<EntityProjectile*> projectiles)
{
	// Sets the list of projectiles
	this->projectiles = projectiles;
	numberOfProjectiles = projectiles.size();
}

EntityProjectile* EntityManagerProjectile::GetProjectile()
{
	// Get the current projectile
	EntityProjectile* projectile = projectiles[currentIndex];

	// Increments index to the next available projectile
	currentIndex = (currentIndex + 1) % numberOfProjectiles;

	// Returns projectile to be used.
	return projectile;
}

void EntityManagerProjectile::Update(float deltaTime, float totalTime)
{
}
