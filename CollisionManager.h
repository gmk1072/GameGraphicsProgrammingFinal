#pragma once
#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "Collider.h"
#include "Entity.h"


class Grid
{
public:
	Grid();
	Grid(float maxScale, XMFLOAT3 halfWidth);
	~Grid();

	int cols = 1;
	XMFLOAT3 halfWidth = XMFLOAT3(10, 10, 10);
	std::unordered_map<int, std::list<void(*)>> grid;

	std::unordered_map<int, std::list<void(*)>>& getMapRef();
	void clear();
	void insert(XMFLOAT3 colLoc, XMFLOAT3 colHalf, void(*colAddress));
};

class CollisionManager
{
public:
	// Instance specific stuff
	static CollisionManager * const Initialize(float maxScale, XMFLOAT3 gridHalfWidth);
	static CollisionManager * const Instance();
	static void Shutdown();

	void StageCollider(Collider* const c);
	void UnstageCollider(Collider* const c);
	void CollisionUpdate();
private:
	CollisionManager(float maxScale, XMFLOAT3 gridHalfWidth);
	~CollisionManager();
	static CollisionManager* instance;
	Grid grid;
	void CollisionInit();
	std::vector<Collider*> colliderVector;
	XMFLOAT3 collisionPoint;

	//typedefs
	typedef bool (CollisionManager::*collisionFunction)(const Collider&, const Collider&);
	typedef std::pair<Collider::ColliderType, Collider::ColliderType> collisionPair;
	//typedef XMFLOAT3 radialVector(const Collider& a, const XMFLOAT3& axis);

	//	Collider-pair jump table
	struct ColliderHasher {
		template <typename C>
		std::size_t operator()(const std::pair<C, C>& pair) const {
			return pair.first * 4 + pair.second;
			//4 collider types
		}
	};
	std::unordered_map<collisionPair, collisionFunction, ColliderHasher> collisionTable;

	//	radial projection jump table
	std::unordered_map<Collider::ColliderType, XMFLOAT3(CollisionManager::*)(const Collider&, const XMFLOAT3&)> radialProjections;

	XMFLOAT3 radialSphere(const Collider& a, const XMFLOAT3& axis);
	XMFLOAT3 radialAABB(const Collider& a, const XMFLOAT3& axis);
	XMFLOAT3 radialOBB(const Collider& a, const XMFLOAT3& axis);
	XMFLOAT3 radialHalfVol(const Collider& a, const XMFLOAT3& axis);

	//		testAxis function
	bool testAxis(const XMFLOAT3& aCenter, const XMFLOAT3& aRad, const XMFLOAT3& bCenter, const XMFLOAT3& bRad, const XMFLOAT3& axis);
	bool testAxis(const Collider& a, const Collider& b, XMFLOAT3 axis);

	//		calc nearest point on OBB
	XMFLOAT3 nearPtOBB(const Collider & obb, XMFLOAT3 axisToC);
	XMFLOAT3 nearPtAABB(const Collider& aabb, XMFLOAT3 axis);
	XMFLOAT3 nearPtPlane(const Collider& plane, const Collider& other);

	//		collision checks
	bool collidesAABBvAABB(const Collider & a, const Collider & b);
	bool collidesSpherevSphere(const Collider & a, const Collider & b);
	bool collidesAABBvSphere(const Collider & a, const Collider & b);
	bool collidesSpherevAABB(const Collider & a, const Collider & b);
	bool collidesOBBvOBB(const Collider & a, const Collider & b);
	bool collidesOBBvSphere(const Collider & a, const Collider & b);
	bool collidesSpherevOBB(const Collider & a, const Collider & b);
	bool collidesOBBvAABB(const Collider & a, const Collider & b);
	bool collidesAABBvOBB(const Collider & a, const Collider & b);
	bool collidesHalfvolvCollider(const Collider & a, const Collider & b);
	bool collidesCollidervHalfvol(const Collider & a, const Collider & b);

	//		collides
	bool collides(const Collider& a, const Collider& b);
};


