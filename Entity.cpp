#include "Entity.h"
#include "EntityFactory.h"
#include "MemoryDebug.h"

// --------------------------------------------------------
// Constructor
//
// Creates an entity with a mesh.
//
// mesh - The mesh this entity will represent
// --------------------------------------------------------
Entity::Entity(EntityFactory* entityFactory, std::string name, Mesh* mesh, Material* material) :
	entityFactory(entityFactory),
	name(name),
	mesh(mesh),
	material(material)
{
	// True when the entity can be rendered
	SetIsRendering(mesh != nullptr && material != nullptr);
	SetIsUpdating(true);
	SetIsColliding(false);
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
Entity::~Entity()
{
	if (collider != nullptr) delete collider;
}

/*
void Entity::PrepareMaterial(SimpleVertexShader* const vertexShader)
{
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	//vertexShader->SetMatrix4x4("world", transform.GetWorldMatrix());
	//vertexShader->SetMatrix4x4("inverseTransposeWorld",
	//	transform.GetInverseTransposeWorldMatrix());

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	//vertexShader->CopyAllBufferData();

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	//vertexShader->SetShader();
	//pixelShader->SetShader();
}
*/

void Entity::SetEntityFactory(EntityFactory* entityFactory)
{
	this->entityFactory = entityFactory;
}

void Entity::SetIsUpdating(bool isUpdating)
{
	// Adds/Removes entity from the list of updating entities
	entityFactory->SetEntityUpdating(this, isUpdating);
}

void Entity::SetIsRendering(bool isRendering)
{
	// Adds/Removes entity from the list of rendering entities
	entityFactory->SetEntityRendering(this, isRendering);
}

void Entity::SetIsColliding(bool isColliding)
{
	// Adds/Removes entity from the list of collidable entities
	entityFactory->SetEntityCollision(this, isColliding);
}

bool Entity::GetIsUpdating()
{
	return isUpdating;
}

bool Entity::GetIsRendering()
{
	return isRendering;
}

bool Entity::GetIsColliding()
{
	return isColliding;
}

// --------------------------------------------------------
// Set the current mesh of this entity
//
// mesh - pointer to mesh
// --------------------------------------------------------
void Entity::SetMesh(Mesh* mesh)
{
	// Ensure new mesh is not null
	assert(mesh != nullptr);
	this->mesh = mesh;
}


// --------------------------------------------------------
// Set the current material of this entity
//
// material - pointer to material
// --------------------------------------------------------
void Entity::SetMaterial(Material* material)
{
	// TODO: Ensure not in process of rendering at this point, i.e. block until next frame
	// Remove self from renderer current bin
	Renderer::Instance()->UnstageEntity(this);

	// Set new material
	this->material = material;

	// Restage to new location
	Renderer::Instance()->StageEntity(this);
}

// --------------------------------------------------------
// Get a constant pointer to this entities current mesh
// --------------------------------------------------------
Mesh * const Entity::GetMesh() const
{
	return mesh;
}

// --------------------------------------------------------
// Get a constant pointer to this entities current material
// --------------------------------------------------------
Material * const Entity::GetMaterial() const
{
	return material;
}

void Entity::SetCollider(Collider::ColliderType type, XMFLOAT3 scale, XMFLOAT3 offset, XMFLOAT4 rotation)
{
	// Check if the entity already has a collider (an existing collider can be edited directly)
	if (collider != nullptr) {
		return;
	}
	// Creates collider object
	collider = new Collider(type, offset, scale, rotation);
	collider->SetParentEntity(this);

	// Set the entity as collidable
	entityFactory->SetEntityCollision(this, true);
}

void Entity::SetName(std::string name)
{
	this->name = name;
}

Collider * const Entity::GetCollider() const
{
	return collider;
}

std::string Entity::GetName() const
{
	return this->name;
}

bool Entity::HasTag(std::string tag)
{
	return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

void Entity::AddTag(std::string tag)
{
	this->tags.push_back(tag);
}

void Entity::RemoveTag(std::string tag)
{
	auto iter = std::find(tags.begin(), tags.end(), tag);
	
	if (iter != tags.end())
		tags.erase(iter);
}

void Entity::OnCollision(Collision collision)
{
}
