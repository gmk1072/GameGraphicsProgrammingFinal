#include "Transform.h"
#include "MemoryDebug.h"

// --------------------------------------------------------
// Constructor
//
// Creates a transform with position of 0, 0, 0, scale of 
// 1, 1, 1 and rotation of 0, 0, 0, 1 (quaternion identity)
//
// --------------------------------------------------------
Transform::Transform() :
	position(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	rotation(0.0f, 0.0f, 0.0f, 1.0f),
	isDirty(0)
{
	// calc initial world mat
	CalculateWorldMatrix();

	// calc init forward, right, ups
	CalculateRightUpForward();

	// calc inverse transpose
	CalculateInverseTransposeWorldMatrix();
}

// --------------------------------------------------------
// Constructor
//
// Creates a transform with supplied initial transform info
//
// position - XMFLOAT3 for position
// scale	- XMFLOAT3 for scale
// rotation	- XMFLOAT4 for quaternion
// --------------------------------------------------------
Transform::Transform(XMFLOAT3 & position,
	XMFLOAT3 & scale,
	XMFLOAT4 & rotation) :
	position(position),
	scale(scale),
	rotation(rotation),
	isDirty(0)
{
	// calc initial world mat
	CalculateWorldMatrix();

	// calc init forward, right, ups
	CalculateRightUpForward();

	// calc inverse transpose
	CalculateInverseTransposeWorldMatrix();
}

// --------------------------------------------------------
// Constructor
//
// Creates a transform with supplied initial transform info
//
// position - float array of 3 elements for position
// scale	- float array of 3 elements for scale
// rotation	- float array of 4 elements for quaternion
// --------------------------------------------------------
Transform::Transform(float position[3],
	float scale[3],
	float rotation[4]) :
	position(position),
	scale(scale),
	rotation(rotation),
	isDirty(0)
{
	// calc initial world mat
	CalculateWorldMatrix();

	// calc init forward, right, ups
	CalculateRightUpForward();

	// calc inverse transpose
	CalculateInverseTransposeWorldMatrix();
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
Transform::~Transform()
{
}

// --------------------------------------------------------
// Move the position vector by some delta
//
// dx - how much to move in the x direction
// dy - how much to move in the y direction
// dz - how much to move in the z direction
// --------------------------------------------------------
void Transform::Move(float dx, float dy, float dz)
{
	position.x += dx;
	position.y += dy;
	position.z += dz;
	isDirty |= (IS_DIRTY_WVM | IS_DIRTY_IVT);
}

// --------------------------------------------------------
// Move the position vector by some delta according to
// current direction vectors
//
// dright	 - how much to move in the right direction
// dup		 - how much to move in the up direction
// dfwd		 - how much to move in the forward direction
// --------------------------------------------------------
void Transform::MoveForward(float dright, float dup, float dfwd)
{
	// Load directions and pos into XMVECTORS
	XMVECTOR xmRight = XMLoadFloat3(&right);
	XMVECTOR xmUp = XMLoadFloat3(&up);
	XMVECTOR xmForward = XMLoadFloat3(&forward);
	XMVECTOR xmPos = XMLoadFloat3(&position);

	// find out how far in each dir we should move
	xmRight = XMVectorScale(xmRight, dright);
	xmUp = XMVectorScale(xmUp, dup);
	xmForward = XMVectorScale(xmForward, dfwd);

	// add up and add to position
	xmPos += xmRight + xmForward + xmUp;

	// Store back in pos and set dirty
	XMStoreFloat3(&position, xmPos);
	isDirty |= (IS_DIRTY_WVM | IS_DIRTY_IVT);
}


// --------------------------------------------------------
// Set the position of this transform
//
// position - array of 3 floats
// --------------------------------------------------------
void Transform::SetPosition(XMFLOAT3 position)
{
	this->position = position;

	// This seems pretty unsafe!
	//memcpy(&this->position, position, sizeof(XMFLOAT3));

	// setting dirty since next time we ask for world, we need to recalc
	isDirty |= (IS_DIRTY_WVM | IS_DIRTY_IVT);
}

// --------------------------------------------------------
// Set the position of this transform
//
// x - x position
// y - y position
// z - z position
// --------------------------------------------------------
void Transform::SetPosition(float x, float y, float z)
{
	// Set XYZ individually
	position.x = x;
	position.y = y;
	position.z = z;

	// setting dirty since next time we ask for world, we need to recalc
	isDirty |= (IS_DIRTY_WVM | IS_DIRTY_IVT);
}

// --------------------------------------------------------
// Set the scale of this transform
//
// scale - array of 3 floats
// --------------------------------------------------------
void Transform::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
	// This seems pretty unsafe!
	//memcpy(&this->scale, scale, sizeof(float) * 3);

	// setting dirty since next time we ask for world, we need to recalc
	isDirty |= (IS_DIRTY_WVM | IS_DIRTY_IVT);
}

// --------------------------------------------------------
// Set the scale of this transform
//
// x - x scale
// y - y scale
// z - z scale
// --------------------------------------------------------
void Transform::SetScale(float x, float y, float z)
{
	// Set XYZ individually
	scale.x = x;
	scale.y = y;
	scale.z = z;

	// setting dirty since next time we ask for world, we need to recalc
	isDirty |= (IS_DIRTY_WVM | IS_DIRTY_IVT);
}

// --------------------------------------------------------
// Set the rotation of this transform
// The given array is interpreted as a quaternion
//
// rotation - array of 4 floats
// --------------------------------------------------------
void Transform::SetRotation(XMFLOAT4 rotation)
{
	this->rotation = rotation;

	// This seems pretty unsafe!
	//memcpy(&this->rotation, rotation, sizeof(float) * 4);

	// setting dirty since next time we ask for world, we need to recalc
	isDirty |= IS_DIRTY_ALL;
}

// --------------------------------------------------------
// Set the rotation of this transform by providing an axis
// and angle
//
// x		- x axis direction
// y		- y axis direction
// z		- z axis direction
// theta	- angle to rotate by
// --------------------------------------------------------
void Transform::SetRotation(float x, float y, float z, float theta)
{
	// Perform angle axis since thats easier to work with
	XMFLOAT3 axis(x, y, z);
	XMStoreFloat4(
		&rotation,
		XMQuaternionRotationAxis(
			XMLoadFloat3(&axis), theta));

	// setting dirty since next time we ask for world, we need to recalc
	isDirty |= IS_DIRTY_ALL;
}

// --------------------------------------------------------
// Get the current position as a an array of floats
// --------------------------------------------------------
const XMFLOAT3 * const Transform::GetPosition() const
{
	// allowed according to http://stackoverflow.com/questions/29277949/casting-a-struct-to-an-array
	// Return position as float array
	return &position;
}

// --------------------------------------------------------
// Get the current scale as a an array of floats
// --------------------------------------------------------
const XMFLOAT3 * const Transform::GetScale() const
{
	// Return scale as float array
	return &scale;
}

// --------------------------------------------------------
// Get the current rotation quaternion as a an array of floats
// --------------------------------------------------------
const XMFLOAT4 * const Transform::GetRotation() const
{
	// Return rotation quaternion as float array
	return &rotation;
}

// --------------------------------------------------------
// Get the right facing direction of this transform
// --------------------------------------------------------
const XMFLOAT3 * const Transform::GetRight()
{
	if (isDirty & IS_DIRTY_RUF)
		CalculateRightUpForward();
	return &right;
}

// --------------------------------------------------------
// Get the up facing direction of this transform
// --------------------------------------------------------
const XMFLOAT3 * const Transform::GetUp()
{
	if (isDirty & IS_DIRTY_RUF)
		CalculateRightUpForward();
	return &up;
}

// --------------------------------------------------------
// Get the forward facing direction of this transform
// --------------------------------------------------------
const XMFLOAT3 * const Transform::GetForward()
{
	if (isDirty & IS_DIRTY_RUF)
		CalculateRightUpForward();
	return &forward;
}

// --------------------------------------------------------
// Get the current dirty status of this transform
// --------------------------------------------------------
const unsigned short Transform::IsDirty() const
{
	return isDirty;
}

// --------------------------------------------------------
// Get the world matrix
// --------------------------------------------------------
const XMFLOAT4X4& Transform::GetWorldMatrix()
{
	// check if we need a recalc
	if (isDirty & IS_DIRTY_WVM)
		CalculateWorldMatrix();
	return world;
}

// --------------------------------------------------------
// Get inverse transpose of the world matrix
// --------------------------------------------------------
const XMFLOAT4X4 & Transform::GetInverseTransposeWorldMatrix()
{
	// if world matrix is dirty, recalc that first
	if (isDirty & IS_DIRTY_WVM)
		CalculateWorldMatrix();

	// check if we need to recalc inv trans
	if (isDirty & IS_DIRTY_IVT)
		CalculateInverseTransposeWorldMatrix();

	return worldInverseTranspose;
}

// --------------------------------------------------------
// Recalculates the current world matrix according to the supplied
// scale, rotation and translation vectors
// --------------------------------------------------------
void inline Transform::CalculateWorldMatrix()
{
	// load scale, rotation and transformation
	XMMATRIX scaleMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
	XMMATRIX rotMat = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
	XMMATRIX transMat = XMMatrixTranslationFromVector(XMLoadFloat3(&position));

	// Calculate world mat
	XMMATRIX worldMat = scaleMat * rotMat * transMat;

	// Store transposed version for use by shader
	XMStoreFloat4x4(&world, XMMatrixTranspose(worldMat));

	// No longer dirty
	isDirty &= (~IS_DIRTY_WVM);
}

// --------------------------------------------------------
// Recalculates the right up and forward vectors
// --------------------------------------------------------
void inline Transform::CalculateRightUpForward()
{
	// Load quat
	XMVECTOR quat = XMLoadFloat4(&rotation);

	// Quat to rot matrix
	XMMATRIX dirs = XMMatrixRotationQuaternion(quat);

	// Load into forward, up, right
	// Daily reminder this is probably really unsafe...
	XMStoreFloat3x3(reinterpret_cast<XMFLOAT3X3*>(&right), dirs);

	// WARNING: IF FORWARD FLIPS OUT, CONSIDER SAFER METHOD BELOW THIS LINE
	//XMStoreFloat3(&right, dirs.r[0]);
	//XMStoreFloat3(&up, dirs.r[1]);
	//XMStoreFloat3(&forward, dirs.r[2]);

	// No longer dirty
	isDirty &= (~IS_DIRTY_RUF);
}

// --------------------------------------------------------
// Recalculates the inverse transpose of world matrix
// --------------------------------------------------------
void inline Transform::CalculateInverseTransposeWorldMatrix()
{
	// Load world matrix
	XMMATRIX worldMat = XMLoadFloat4x4(&world);

	// inverse then transpose
	XMMATRIX result = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMat));

	// store
	XMStoreFloat4x4(&worldInverseTranspose, result);

	// undirty
	isDirty &= (~IS_DIRTY_IVT);
}
