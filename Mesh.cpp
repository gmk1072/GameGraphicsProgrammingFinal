#include "Mesh.h"
#include "MemoryDebug.h"

using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// Creates a mesh by uploading supplied data to the GPU.
//
// vertices - array of vertices to upload
// numVerts - number of vertices in the array
// indices	- array of indices to upload, will be used for indexed rendering
// numIndices - number of indices in the array
// device	- device to upload our data to
// --------------------------------------------------------
Mesh::Mesh(
	Vertex* const vertices,
	const int numVerts,
	unsigned int* const indices,
	const int numIndices,
	ID3D11Device* const device)
{
	MeshParameters params = { vertices, indices, numVerts, numIndices };
	if (!UploadModel(params, device))
		fprintf(stderr, "ERROR: Failed to upload model from ctor!\n");
}


// --------------------------------------------------------
// Constructor
//
// Creates a mesh by loading in an OBJ file and uploading 
// its vertices and other information to the GPU
//
// file		- file path to obj file
// device	- device to upload our data to
// --------------------------------------------------------
Mesh::Mesh(const char * const file, ID3D11Device * const device)
{
	if(file)
	LoadFBX(file, device); //actually loads many types
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
Mesh::~Mesh()
{
	// Free DX resources
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }
}

// --------------------------------------------------------
// Get a constant pointer to the vertex buffer of this mesh
// --------------------------------------------------------
ID3D11Buffer * const Mesh::GetVertexBuffer() const
{
	return vertexBuffer;
}

// --------------------------------------------------------
// Get a constant pointer to the index buffer of this mesh
// --------------------------------------------------------
ID3D11Buffer * const Mesh::GetIndexBuffer() const
{
	return indexBuffer;
}

// --------------------------------------------------------
// Get the number of indices in the index buffer for this mesh
// --------------------------------------------------------
const int Mesh::GetIndexCount() const
{
	return numIndices;
}


// --------------------------------------------------------
// Loads an OBJ file onto the stack then uploads the model
// data to the GPU.
//
// objFile	- file path to obj file
// device	- device to upload our data to
// --------------------------------------------------------
void Mesh::LoadOBJ(const char * const objFile, ID3D11Device* const device)
{
	// Basic precondition checks
	assert(objFile != nullptr);
	assert(device != nullptr);

	// Init params as 0
	Mesh::MeshParameters meshParams = {};

	// File input object
	std::ifstream obj(objFile);

	// Check for successful open
	if (!obj.is_open())
	{
		fprintf(stderr, "ERROR: Model not found %s\n", objFile);
		return;
	}

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

										 // Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			memset(&v1, 0, sizeof(Vertex));
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			memset(&v2, 0, sizeof(Vertex));
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			memset(&v3, 0, sizeof(Vertex));
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				memset(&v4, 0, sizeof(Vertex));
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	// - At this point, "verts" is a vector of Vertex structs, and can be used
	//    directly to create a vertex buffer:  &verts[0] is the address of the first vert
	//
	// - The vector "indices" is similar. It's a vector of unsigned ints and
	//    can be used directly for the index buffer: &indices[0] is the address of the first int
	//
	// - "vertCounter" is BOTH the number of vertices and the number of indices
	// - Yes, the indices are a bit redundant here (one per vertex)

	// set mesh params
	meshParams.vertices = &verts[0];
	meshParams.indices = &indices[0];
	meshParams.numVerts = vertCounter;
	meshParams.numIndices = vertCounter;

	// Calculate tangents
	CalculateTangents(&verts[0], vertCounter, &indices[0], vertCounter);

	// upload model
	UploadModel(meshParams, device);
}

// --------------------------------------------------------
// Loads an FBX file onto the stack then uploads the model
// data to the GPU.
//
// fbxFile	- file path to obj file
// device	- device to upload our data to
// --------------------------------------------------------
void Mesh::LoadFBX(const char * const fbxFile, ID3D11Device* const device)
{
	// Basic precondition checks
	assert(fbxFile != nullptr);
	assert(device != nullptr);

	// Init params as 0
	Mesh::MeshParameters meshParams = {};

	//importer to read the fbx file
	Assimp::Importer imp = Assimp::Importer();
	const aiScene* scene = imp.ReadFile(fbxFile, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_SortByPType);

	// Check for successful open
	if (scene == NULL)
	{
		fprintf(stderr, "ERROR: Model not found %s\n", fbxFile);
		return;
	}


	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices

	//load mesh
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
											 
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			//positions
			aiVector3D pos = mesh->mVertices[i];
			//transfer to XMFLOAT
			float x = pos.x;
			float y = pos.y;
			float z = pos.z;
			XMFLOAT3 posDX = XMFLOAT3(x, y, z);

			//normals
			aiVector3D norm = mesh->mNormals[i];
			//transfer to XMFLOAT
			x = norm.x;
			y = norm.y;
			z = norm.z;
			XMFLOAT3 normDX = XMFLOAT3(x, y, z);

			//uvs
			aiVector3D uvw = mesh->mTextureCoords[0][i];
			//transfer to XMFLOAT
			x = uvw.x;
			y = uvw.y;
			XMFLOAT2 uvDX = XMFLOAT2(x, y);

			//add to lists
			positions.push_back(posDX);
			normals.push_back(normDX);
			uvs.push_back(uvDX);
		}

		//TODO - fix this, i think the problem is here but i could be wrong
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
			//should always be 3 thanks to post processing
			if (face.mNumIndices == 3) {
				Vertex v1;
				memset(&v1, 0, sizeof(Vertex));
				v1.Position = positions[face.mIndices[0]];
				v1.UV = uvs[face.mIndices[0]];
				v1.Normal = normals[face.mIndices[0]];

				Vertex v2;
				memset(&v2, 0, sizeof(Vertex));
				v2.Position = positions[face.mIndices[1]];
				v2.UV = uvs[face.mIndices[1]];
				v2.Normal = normals[face.mIndices[1]];

				Vertex v3;
				memset(&v3, 0, sizeof(Vertex));
				v3.Position = positions[face.mIndices[2]];
				v3.UV = uvs[face.mIndices[2]];
				v3.Normal = normals[face.mIndices[2]];

				// Add the verts to the vector (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v3);
				verts.push_back(v2);

				// Add indices
				indices.push_back(face.mIndices[0]); vertCounter++;
				indices.push_back(face.mIndices[1]); vertCounter++;
				indices.push_back(face.mIndices[2]); vertCounter++;
			}
		}

		//not implemented yet
		// !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! 
		// CALC TANGENTS CALLED BELOW FOR NOW, REMOVE FUNCTION CALL WHEN THIS
		// IS IMPLEMENTED
		// !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! 
		/*
		for (int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D tan = mesh->mTangents[i];

			//transfer to XMFLOAT
			float x = tan.x;
			float y = tan.y;
			float z = tan.z;
			XMFLOAT3 tanDX = XMFLOAT3(x, y, z);
			//tangents.push_back(normDX);
		}
		/**/
	}

	// set mesh params
	meshParams.vertices = &verts[0];
	meshParams.indices = &indices[0];
	meshParams.numVerts = vertCounter;
	meshParams.numIndices = vertCounter;

	// Calc tangents (remove this later)
	CalculateTangents(&verts[0], vertCounter, &indices[0], vertCounter);

	// upload model
	UploadModel(meshParams, device);
}

// Calculates the tangents of the vertices in a mesh
// Code adapted from: http://www.terathon.com/code/tangent.html
void Mesh::CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices)
{
	// Reset tangents
	for (int i = 0; i < numVerts; i++)
	{
		verts[i].Tangent = XMFLOAT3(0, 0, 0);
	}

	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < numVerts;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = indices[i++];
		unsigned int i2 = indices[i++];
		unsigned int i3 = indices[i++];
		Vertex* v1 = &verts[i1];
		Vertex* v2 = &verts[i2];
		Vertex* v3 = &verts[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx;
		v1->Tangent.y += ty;
		v1->Tangent.z += tz;

		v2->Tangent.x += tx;
		v2->Tangent.y += ty;
		v2->Tangent.z += tz;

		v3->Tangent.x += tx;
		v3->Tangent.y += ty;
		v3->Tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < numVerts; i++)
	{
		// Grab the two vectors
		XMVECTOR normal = XMLoadFloat3(&verts[i].Normal);
		XMVECTOR tangent = XMLoadFloat3(&verts[i].Tangent);

		// Use Gram-Schmidt orthogonalize
		tangent = XMVector3Normalize(
			tangent - normal * XMVector3Dot(normal, tangent));

		// Store the tangent
		XMStoreFloat3(&verts[i].Tangent, tangent);
	}
}

// --------------------------------------------------------
// Uploads provided model data to the GPU.
//
// params	- Required mesh parameters for uploading data
// device	- device to upload our data to
// --------------------------------------------------------
bool Mesh::UploadModel(const MeshParameters& params, ID3D11Device* const device)
{
	// Basic foolproof checks
	// Ensure minimal params passed
	if (device == nullptr ||
		params.indices == nullptr ||
		params.vertices == nullptr ||
		params.numIndices <= 0 ||
		params.numVerts <= 0)
		return false;
	// if any of the struct 
	//assert(vertices != nullptr);
	//assert(indices != nullptr);
	//assert(device != nullptr);
	//assert(numVerts > 0 && numIndices > 0);

	// Init fields
	this->numIndices = params.numIndices;
	vertexBuffer = nullptr;
	indexBuffer = nullptr;

	// Create the VERTEX BUFFER description -----------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * params.numVerts; // multiply number of verts by size of vertex so GPU knows exact num of bytes
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Create the proper struct to hold the initial vertex data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = params.vertices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	// Create the INDEX BUFFER description ------------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * numIndices; // multiply num of indices by sizeof int so GPU knows exact num of bytes for array
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Create the proper struct to hold the initial index data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = params.indices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);

	return true;
}
