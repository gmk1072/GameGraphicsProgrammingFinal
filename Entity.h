#pragma once
#include "Mesh.h"
#include "Material.h"
#include "MaterialParallax.h"
#include "Transform.h"
#include "Collider.h"
#include "Renderer.h"

class Renderer; 
class EntityFactory;

// Created on a collision of two entities
struct Collision {
	Entity* otherEntity;
	Collider* otherCollider;
	Transform otherTransform;
	XMFLOAT3 point;
};

// Base type of all entities.
// Entities can update every frame, render a mesh, and be collidable.
class Entity
{
	friend EntityFactory;

public:
	// Construction of an entity requires a mesh and material
	Entity(
		EntityFactory* entityFactory,
		std::string name,
		Mesh* mesh = nullptr,
		Material* material = nullptr
	);
	virtual ~Entity();

	// must be implemented in class which extends this one
	virtual void Update(float deltaTime, float totalTime) = 0;
	//void PrepareMaterial(SimpleVertexShader* const vertexShader);

	// Called when this entity collides with another entity.
	virtual void OnCollision(Collision collision);

	// Public transform so we can access information!
	Transform transform;

	// Getters and Setters
	void SetEntityFactory(EntityFactory* entityFactory);

	void SetIsUpdating(bool isUpdating);	// Sets if the entity updates every frame
	void SetIsRendering(bool isRendering);	// Sets if the entity is rendered
	void SetIsColliding(bool isColliding);	// Sets if the entity has collision

	bool GetIsUpdating();	// Returns true if the entity is updated every frame
	bool GetIsRendering();	// Returns true if the entity is rendered every frame
	bool GetIsColliding();	// Returns true if the entity has collision enabled.

	void SetMesh(Mesh* mesh);
	void SetMaterial(Material* material);
	void SetCollider(Collider::ColliderType type, XMFLOAT3 scale = XMFLOAT3(0, 0, 0), XMFLOAT3 offset = XMFLOAT3(0, 0, 0), XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 0));
	void SetName(std::string name);
	Mesh * const GetMesh() const;
	Material * const GetMaterial() const;
	Collider * const GetCollider() const;
	std::string GetName() const;

	// Tags - Used mainly to identify during collisions
	bool HasTag(std::string tag);
	void AddTag(std::string tag);
	void RemoveTag(std::string tag);

private:
	// Pointer to the entity factory that holds this entity
	EntityFactory* entityFactory;

	// Properties
	bool isUpdating = false;
	bool isRendering = false;
	bool isColliding = false;

	// Identifiers
	std::string name;
	std::vector<std::string> tags;

	// Mesh class this entity will draw with
	Mesh* mesh = nullptr;
	
	// Material containing shaders that this entity will draw with
	Material* material = nullptr;

	// Collider for object
	Collider* collider = nullptr;
};

