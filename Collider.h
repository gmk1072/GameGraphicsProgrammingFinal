#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Transform.h"
#include "Mesh.h"

class Entity;

class Collider
{
public:
	//types of colliders
	enum ColliderType { OBB, AABB, SPHERE, HALFVOL };

	//0'd offset and rotation, 1 scale
	Collider();
	Collider(ColliderType type);
	// Initialize with given vars
	Collider(ColliderType type,
		XMFLOAT3 &offset,
		XMFLOAT3 &scale,
		XMFLOAT4 &rotation);

	// Initialize with given vars
	Collider(ColliderType type,
		float offset[3],
		float scale[3],
		float rotation[4]);
	// Initialize based on mesh size
	Collider(ColliderType type,
		Mesh* mesh);

	~Collider();

	const XMFLOAT3* const GetOffset() const;
	const XMFLOAT3* const GetScale() const;
	float const GetMaxScale() const;
	const XMFLOAT4* const GetRotation() const;
	ColliderType const GetType() const;
	XMFLOAT4X4 GetRotationMatrix()const;
	XMFLOAT3 const GetPosition() const;
	XMFLOAT4 const GetRotationColumn(int col) const;

	void SetOffset(XMFLOAT3 offIn);
	void SetScale(XMFLOAT3 scaleIn);

	//Part of all components
	void SetParentEntity(Entity* parent);
	Entity* const GetParentEntity() const;
	const Entity* const GetBaseEntity() const;	//same as GetParentEntity unless heirarchy of entities is implemented

private:
	XMFLOAT3 offset; // vec3
	XMFLOAT3 scale; // vec3
	XMFLOAT4 rotation; // quat

	ColliderType colType;
	Entity* parentEntity;

	XMFLOAT4 GetEntityRotation() const;
};

