#include "EntityStatic.h"
#include "MemoryDebug.h"

EntityStatic::EntityStatic(EntityFactory* entityFactory, std::string name, Mesh * mesh, Material * material) : 
	Entity(entityFactory, name, mesh, material)
{
	SetIsUpdating(false);	// Static entities do not update
}


EntityStatic::~EntityStatic()
{
}

void EntityStatic::Update(float deltaTime, float totalTime)
{
}
