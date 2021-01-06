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
	uint fbxSkinIndex : 8;

	FBXLoadOptionChunk() : flags(0), sampleFPS(60), fbxSkinIndex(0) { }
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
		Vector3f* vertices;
		Vector3f* normals;
		Vector3f* tangents;
		Vector3f* binormals;
		uint uvSlotCount;
		Vector2f** uvSlots;
		Vector4i* boneIndices;
		Vector4f* boneWeights;
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
	char* name;
	int parentIndex;
	int childIndexStart;
	int childCount;
	Matrix4x4 inverseGlobalTransformMatrix;
	TRS localTransform;
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
	FBXHierarchyNode* hierarchyNodes;

	// animation
	uint animationCount;
	struct FBXAnimation
	{
		char* animationName;
		uint frameKeyCount;
		uint fpsCount;
		Matrix4x4* globalAffineTransforms;
	}* animations;

	// allocater
	const Allocaters* allocs;

	FBXChunk() :
		meshCount(0), meshs(nullptr), materialPropertyCount(0), textureRefs(0),
		hierarchyCount(0), hierarchyNodes(nullptr), animationCount(0), animations(nullptr), 
		allocs(nullptr)
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
				SAFE_DEALLOC(mesh->geometry.boneIndices, allocs->dealloc);
				SAFE_DEALLOC(mesh->geometry.boneWeights, allocs->dealloc);

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
		SAFE_DEALLOC(hierarchyNodes, allocs->dealloc);
		if (animations)
		{
			for (uint i = 0; i < animationCount; i++)
				SAFE_DEALLOC(animations[i].globalAffineTransforms, allocs->dealloc);

			allocs->dealloc(animations);
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
