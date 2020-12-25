#include <functional>
#include "datatypes.h"

#pragma once

struct FBXLoadOptionChunk
{
	union
	{
		struct
		{
			uint flipface : 1;
			uint flipU : 1;
			uint flipV : 1;
			uint fillVertexNomral : 1;
			uint fillVertexTangent : 1;
			uint fillVertexBinormal : 1;
			uint padding : 2;
		};
		byte flags;
	};
	uint sampleFPS : 8;

	FBXLoadOptionChunk() : flags(0), sampleFPS(60) { }
};

struct FBXPivotChunk
{
	double prevRotation[3];
	double postRotation[3];
	double rotationPivot[3];
	double rotationOffset[3];
	double scalingPivot[3];
	double scalingOffset[3];
};

struct FBXMeshChunk
{
	char* name;
	struct FBXGeometryChunk
	{
		uint vertexCount;
		Vector4f* vertices;
		Vector3f* normals;
		Vector3f* tangents;
		Vector3f* binormals;
		uint uvSlotCount;
		Vector2f** uvSlots;
		uint indexCount;
		uint* indices;
	} geometry;
	struct FBXSubmesh
	{
		uint submeshCount;
		struct Submesh
		{
			uint indexStart;
			uint indexCount;
			uint materialRef;
		}*submeshs;
	} submesh;
};

struct FBXHierarchyNode
{
	int parentIndexOffset;
	int nameStartIndex;
};

struct FBXChunk
{
	// mesh
	uint meshCount;
	FBXMeshChunk* meshs;

	// material
	uint materialPropertyCount;
	uint* textureRefs;

	// hierarchy
	uint hierarchyCount;
	wchar_t* hierarchyNames;
	FBXHierarchyNode* hierarchyNodes;

	// skeleton
	uint skeletonCount;
	int* skeletonHierarchyIndices;

	// animation
	uint frameKeyCount;
	uint fpsCount;
	uint transformCount;
	Matrix4x4** globalAffineTransforms;

	// allocater
	const Allocaters* allocs;

	FBXChunk() :
		meshCount(0), meshs(nullptr), materialPropertyCount(0), textureRefs(0),
		hierarchyCount(0), hierarchyNames(nullptr), hierarchyNodes(nullptr),
		skeletonCount(0), skeletonHierarchyIndices(nullptr),
		frameKeyCount(0), fpsCount(0), transformCount(0), globalAffineTransforms(nullptr), allocs(nullptr)
	{}
	~FBXChunk()
	{
		if (meshs)
		{
			for (uint i = 0; i < meshCount; i++)
			{
				FBXMeshChunk* mesh = meshs + i;

				SAFE_DEALLOC(mesh->geometry.vertices, allocs->dealloc);
				SAFE_DEALLOC(mesh->geometry.normals, allocs->dealloc);
				SAFE_DEALLOC(mesh->geometry.tangents, allocs->dealloc);
				SAFE_DEALLOC(mesh->geometry.binormals, allocs->dealloc);

				SAFE_DEALLOC(mesh->geometry.indices, allocs->dealloc);

				if (mesh->geometry.uvSlots)
				{
					for (uint j = 0; j < mesh->geometry.uvSlotCount; j++)
						SAFE_DEALLOC(mesh->geometry.uvSlots[j], allocs->dealloc);
					allocs->dealloc(mesh->geometry.uvSlots);
				}
				
				SAFE_DEALLOC(mesh->submesh.submeshs, allocs->dealloc);
			}
			allocs->dealloc(meshs);
		}

		SAFE_DEALLOC(textureRefs, allocs->dealloc);
		SAFE_DEALLOC(hierarchyNames, allocs->dealloc);
		SAFE_DEALLOC(hierarchyNodes, allocs->dealloc);
		SAFE_DEALLOC(skeletonHierarchyIndices, allocs->dealloc);

		if (globalAffineTransforms)
		{
			for (uint i = 0; i < frameKeyCount; i++)
				SAFE_DEALLOC(globalAffineTransforms[i], allocs->dealloc);
			allocs->dealloc(globalAffineTransforms);
		}
	}
};

void InitializeFBX();
void DestroyFBX();

bool ImportFBX(
	const wchar_t* fileDirectory, FBXChunk& chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
bool ImportFBX(
	const char* fileDirectory, FBXChunk& chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
uint ImportFBX(
	uint chunkCount, const char* const * fileDirectories, FBXChunk* chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
uint ImportFBX(
	uint chunkCount, const wchar_t* const * fileDirectories, FBXChunk* chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
