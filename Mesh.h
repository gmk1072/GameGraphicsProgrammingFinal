#pragma once

#include <d3d11.h>
#include <vector>
#include <fstream>
#include "Vertex.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

// Should int for size of array be unsigned int, int, or size_t ?
class Mesh
{
public:
	Mesh(
		Vertex* const vertices,		// array of vertices to upload
		const int numVerts,			// number of vertices in the array
		unsigned int* const indices,	// array of indices to upload, will be used for indexed rendering
		const int numIndices,			// number of indices in the array
		ID3D11Device* const device	// device to upload our data to
	);

	Mesh(
		const char* const objFile,		// obj file to load
		ID3D11Device* const device		// device to upload our data to
	);

	~Mesh();

	// Getters for information required to draw our mesh to the screen
	ID3D11Buffer* const GetVertexBuffer() const;
	ID3D11Buffer* const GetIndexBuffer() const;
	const int GetIndexCount() const;

private:
	// Use this struct when passing parameters around
	struct MeshParameters
	{
		Vertex* vertices;		// array of vertices to upload
		unsigned int* indices;	// array of indices to upload, will be used for indexed rendering
		int numVerts;			// number of vertices in the array
		int numIndices;			// number of indices in the array
	};							

	// Model loading funciton
	void LoadOBJ(const char* const objFile, ID3D11Device* const device);
	void LoadFBX(const char* const fbxFile, ID3D11Device* const device);
	void CalculateTangents(Vertex * verts, int numVerts, unsigned int * indices, int numIndices);
	bool UploadModel(const MeshParameters& params, ID3D11Device* const device);

	// Data required in order to draw our mesh to the screen
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	int numIndices;

};

