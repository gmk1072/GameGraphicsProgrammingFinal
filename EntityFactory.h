#pragma once
#include <unordered_map>

// Entities
#include "Entity.h"
#include "EntityPlayer.h"
#include "EntityEnemy.h"
#include "EntityProjectile.h"
#include "EntityStatic.h"
#include "EntityManagerProjectile.h"

// Managers
#include "CollisionManager.h"
#include "Renderer.h"

class Mesh;
class Material;

// Enum of entity types
enum EntityType {
	STATIC,
	ENEMY,
	PLAYER,
	PROJECTILE,
	MANAGER_PROJECTILE,
};

// Creates and manages entities in a scene
// Handles the unloading of entities
class EntityFactory
{
protected:
	 std::unordered_map<std::string, Entity*> entities;
	 std::unordered_map<std::string, Entity*> updatingEntities;
	 
	 CollisionManager* collisionManager;
	 Renderer* renderer;


	 void AddEntityToUpdate(Entity* entity);
	 void RemoveEntityFromUpdate(Entity* entity);

public:
	void Release();
	
	Entity* CreateEntity(EntityType entityType, std::string name, Mesh* mesh = nullptr, Material* material = nullptr);
	std::vector<EntityProjectile*> CreateProjectileEntities(unsigned int numberOfProjectiles, Mesh* mesh = nullptr, Material* material = nullptr);

	void UpdateEntities(float deltaTime, float  totalTime);

	void SetEntityCollision(Entity* entity, bool isColliding);
	void SetEntityRendering(Entity* entity, bool isRendering);
	void SetEntityUpdating(Entity* entity, bool isUpdating);

	std::unordered_map<std::string, Entity*> GetEntities();
};

