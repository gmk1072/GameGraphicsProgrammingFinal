#include "Collider.h"
#include "Entity.h"
#include "MemoryDebug.h"

Collider::Collider() : 
	colType(OBB),
	offset(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	rotation(0.0f, 0.0f, 0.0f, 1.0f)
{}

Collider::Collider(ColliderType type) :
	colType(type),
	offset(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	rotation(0.0f, 0.0f, 0.0f, 1.0f)
{}

Collider::Collider(ColliderType type, XMFLOAT3 & offset, XMFLOAT3 & scale, XMFLOAT4 & rotation) :
	colType(type),
	offset(offset),
	scale(scale),
	rotation(rotation)
{}


Collider::Collider(ColliderType type, float offset[3], float scale[3], float rotation[4]) :
	colType(type),
	offset(offset),
	scale(scale),
	rotation(rotation)
{}

Collider::Collider(ColliderType type, Mesh * mesh) :
	colType(type)
{
	// Ensure we're starting out with a mesh
	assert(mesh != nullptr);

	//calculate offset, scale, and rotation using the mesh data
}

Collider::~Collider()
{
}

// --------------------------------------------------------
// Get the current offset as a an array of floats
// --------------------------------------------------------
const XMFLOAT3 * const Collider::GetOffset() const
{
	return &offset;
}

// --------------------------------------------------------
// Get the current scale as a an array of floats
// --------------------------------------------------------
const XMFLOAT3 * const Collider::GetScale() const
{
	return &scale;
}

// --------------------------------------------------------
// Get the maximum scale value
// --------------------------------------------------------
float const Collider::GetMaxScale() const
{
	return max(abs(scale.x), max(abs(scale.y), abs(scale.z)));
}

// --------------------------------------------------------
// Get the current rotation quaternion as a an array of floats
// --------------------------------------------------------
const XMFLOAT4 * const Collider::GetRotation() const
{
	return &rotation;
}

const Collider::ColliderType Collider::GetType() const
{
	return colType;
}

XMFLOAT4X4 Collider::GetRotationMatrix() const
{
	XMFLOAT4 trans = GetEntityRotation();
	XMFLOAT4 curr = *GetRotation();
	XMVECTOR transQ = XMLoadFloat4(&trans);
	XMVECTOR currQ = XMLoadFloat4(&curr);

	transQ = XMQuaternionMultiply(transQ, currQ);

	//XMStoreFloat4(&trans, transQ);
	XMMATRIX rotMat = XMMatrixRotationQuaternion(transQ);
	XMFLOAT4X4 rot;
	XMStoreFloat4x4(&rot, rotMat);
	return rot;
}

XMFLOAT3 const Collider::GetPosition() const
{
	//TODO: apply rotation of transform to the offset
	XMVECTOR parentLocation = XMLoadFloat3(parentEntity->transform.GetPosition());
	XMVECTOR colliderOffset = XMLoadFloat3(&offset);

	XMFLOAT3 position;
	XMStoreFloat3(&position, parentLocation + colliderOffset);
	return position;
}


XMFLOAT4 const Collider::GetRotationColumn(int col) const
{
	if (col < 0 || col > 3) {
		//out of index
		printf("Out of range for column search");
		return XMFLOAT4();
	}

	XMFLOAT4X4 rotMat = this->GetRotationMatrix();
	XMFLOAT4 rot;
	switch (col) {
	case 0: 
		rot = XMFLOAT4(rotMat._11, rotMat._21, rotMat._31, rotMat._41);
		break;
	case 1:
		rot = XMFLOAT4(rotMat._12, rotMat._22, rotMat._32, rotMat._42);
		break;
	case 2:
		rot = XMFLOAT4(rotMat._13, rotMat._23, rotMat._33, rotMat._43);
		break;
	case 3:
		rot = XMFLOAT4(rotMat._14, rotMat._24, rotMat._34, rotMat._44);
		break;
	}
	return rot;
}

void Collider::SetOffset(XMFLOAT3 offIn)
{
	offset = offIn;
}

void Collider::SetScale(XMFLOAT3 scaleIn)
{
	scale = scaleIn;
}

void Collider::SetParentEntity(Entity * parent)
{
	parentEntity = parent;
}

Entity * const Collider::GetParentEntity() const
{
	return parentEntity;
}

const Entity * const Collider::GetBaseEntity() const
{
	//alter to allow for parent/child function in Entity itself
	return parentEntity;
}

XMFLOAT4 Collider::GetEntityRotation() const
{
	XMFLOAT4 rot = *(parentEntity->transform.GetRotation());
	return rot;
}

