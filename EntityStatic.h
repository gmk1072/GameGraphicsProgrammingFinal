#pragma once
#include "Entity.h"

// Entity that does nothing but render. Good for background pieces.
class EntityStatic :
	public Entity
{
public:
	EntityStatic(EntityFactory* entityFactory, std::string name, Mesh * mesh, Material * material);
	virtual ~EntityStatic();

	// Inherited via Entity
	virtual void Update(float deltaTime, float totalTime) override;
};

