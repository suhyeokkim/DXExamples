#include <fbxsdk.h>
#include <vector>
#include <queue>
#include <algorithm>

#include "fbximport.h"
#include "datatypes.h"

using namespace fbxsdk;

FbxManager* g_FbxManager = nullptr;

void InitializeFBX()
{
	g_FbxManager = FbxManager::Create();
	FALSE_ERROR_MESSAGE_RETURN_VOID(g_FbxManager, L"fail to create FbxManager..");
	
	FbxIOSettings* fbxIoSettings = FbxIOSettings::Create(g_FbxManager, IOSROOT);
	g_FbxManager->SetIOSettings(fbxIoSettings);
}

void DestroyFBX()
{
	if (g_FbxManager)
	{
		g_FbxManager->Destroy();
		g_FbxManager = nullptr;
	}
}

bool SceneToChunk(FbxScene* fbxScene, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);
bool ImportFBX(const wchar_t* fileDirectory, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	if (g_FbxManager == nullptr)
	{
		InitializeFBX();
		FALSE_ERROR_MESSAGE_RETURN_CODE(g_FbxManager != nullptr, L"fail to initialize fbxmaneger..", false);
	}

	size_t len = lstrlenW(fileDirectory);
	char fileDirBuffer[512];
	wcstombs_s(&len, fileDirBuffer, 512, fileDirectory, len * sizeof(wchar_t));

	FBXLoadOptionChunk dfltopt;
	char fbxObjectName[512];
	sprintf_s(fbxObjectName, "Importer(%s)", fileDirBuffer);
	FbxImporter* fbxImporter = FbxImporter::Create(g_FbxManager, fbxObjectName);
	sprintf_s(fbxObjectName, "Scene(%s)", fileDirBuffer);
	FbxScene* fbxScene = FbxScene::Create(g_FbxManager, fbxObjectName);

	bool importStatus = fbxImporter->Initialize(fileDirBuffer, -1, g_FbxManager->GetIOSettings());
	FALSE_ERROR_MESSAGE_GOTO(importStatus, L"fail to initialize fbximporter..", IMPORTFBX_FAIL);

	importStatus = fbxImporter->Import(fbxScene);
	FALSE_ERROR_MESSAGE_GOTO(
		importStatus && fbxImporter->GetStatus() == FbxStatus::eSuccess,
		L"fail to import scene..",
		IMPORTFBX_FAIL
	);

	importStatus = SceneToChunk(fbxScene, chunk, opt? opt: &dfltopt, allocs);
	FALSE_ERROR_MESSAGE_GOTO(importStatus, L"fail to copy to chunk..", IMPORTFBX_FAIL);

	if (fbxImporter) fbxImporter->Destroy();
	if (fbxScene) fbxScene->Destroy();

	return true;

IMPORTFBX_FAIL:

	return false;
}

bool ImportFBX(const char* fileDirectory, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	if (g_FbxManager == nullptr)
	{
		InitializeFBX();
		FALSE_ERROR_MESSAGE_RETURN_CODE(g_FbxManager != nullptr, L"fail to initialize fbxmaneger..", false);
	}

	FBXLoadOptionChunk dfltopt;
	char fbxObjectName[512];
	sprintf_s(fbxObjectName, "Importer(%s)", fileDirectory);
	FbxImporter* fbxImporter = FbxImporter::Create(g_FbxManager, fbxObjectName);
	sprintf_s(fbxObjectName, "Scene(%s)", fileDirectory);
	FbxScene* fbxScene = FbxScene::Create(g_FbxManager, fbxObjectName);

	bool importStatus = fbxImporter->Initialize(fileDirectory, -1, g_FbxManager->GetIOSettings());
	FALSE_ERROR_MESSAGE_GOTO(importStatus, L"fail to initialize fbximporter..", IMPORTFBX_FAIL);

	importStatus = fbxImporter->Import(fbxScene);
	FALSE_ERROR_MESSAGE_GOTO(
		importStatus && fbxImporter->GetStatus() == FbxStatus::eSuccess,
		L"fail to import scene..",
		IMPORTFBX_FAIL
	);

	importStatus = SceneToChunk(fbxScene, chunk, opt? opt: &dfltopt, allocs);
	FALSE_ERROR_MESSAGE_GOTO(importStatus, L"fail to copy to chunk..", IMPORTFBX_FAIL);

	if (fbxImporter) fbxImporter->Destroy();
	if (fbxScene) fbxScene->Destroy();

	return true;

IMPORTFBX_FAIL:

	return false;
}

uint ImportFBX(uint chunkCount, const char* const * fileDirectories, FBXChunk* chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	uint validChunkCount = 0;
	char fbxObjectName[512];
	FBXLoadOptionChunk dfltopt;

	for (uint i = 0; i < chunkCount; i++)
	{
		sprintf_s(fbxObjectName, "Importer(%s)", fileDirectories[i]);
		FbxImporter* fbxImporter = FbxImporter::Create(g_FbxManager, fbxObjectName);
		sprintf_s(fbxObjectName, "Scene(%s)", fileDirectories[i]);
		FbxScene* fbxScene = FbxScene::Create(g_FbxManager, fbxObjectName);

		bool importStatus = fbxImporter->Initialize(fileDirectories[i], -1, g_FbxManager->GetIOSettings());
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus, L"fail to initialize fbximporter..");

		importStatus = fbxImporter->Import(fbxScene);
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus && fbxImporter->GetStatus() == FbxStatus::eSuccess, L"fail to import scene..");

		importStatus = SceneToChunk(fbxScene, *(chunk + i), opt? opt + i: &dfltopt, allocs);
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus, L"fail to copy to chunk..");

		if (fbxImporter) fbxImporter->Destroy();
		if (fbxScene) fbxScene->Destroy();

		validChunkCount++;
	}

	return validChunkCount;
}

uint ImportFBX(uint chunkCount, const wchar_t* const * fileDirectories, FBXChunk* chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	uint validChunkCount = 0;
	char fileDirBuffer[512], fbxObjectName[512];
	FBXLoadOptionChunk dfltopt;

	for (uint i = 0; i < chunkCount; i++)
	{
		size_t len = wcslen(fileDirectories[i]);
		wcstombs_s(&len, fileDirBuffer, 512, fileDirectories[i], len * sizeof(wchar_t));

		sprintf_s(fbxObjectName, "Importer(%s)", fileDirBuffer);
		FbxImporter* fbxImporter = FbxImporter::Create(g_FbxManager, fbxObjectName);
		sprintf_s(fbxObjectName, "Scene(%s)", fileDirBuffer);
		FbxScene* fbxScene = FbxScene::Create(g_FbxManager, fbxObjectName);

		bool importStatus = fbxImporter->Initialize(fileDirBuffer, -1, g_FbxManager->GetIOSettings());
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus, L"fail to initialize fbximporter..");

		importStatus = fbxImporter->Import(fbxScene);
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus && fbxImporter->GetStatus() == FbxStatus::eSuccess, L"fail to import scene..");

		importStatus = SceneToChunk(fbxScene, *(chunk + i), opt ? opt + i : &dfltopt, allocs);
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus, L"fail to copy to chunk..");

		if (fbxImporter) fbxImporter->Destroy();
		if (fbxScene) fbxScene->Destroy();

		validChunkCount++;
	}

	return validChunkCount;
}

void HierarchyAllocate(FbxScene* fbxScene, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);
uint TraversalFBXNode(FbxNode* node, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);
bool AnimationToChunk(FbxScene* scene, FBXChunk& chunk, const Allocaters* allocs);
bool BlendShapeToChunk(FbxScene* fbxScene, FBXChunk& chunk, const Allocaters* allocs);

int GetFPS(const fbxsdk::FbxTime::EMode& mode)
{
	switch (mode)
	{
	case fbxsdk::FbxTime::EMode::eFrames96:
		return 96;
	case fbxsdk::FbxTime::EMode::eFrames100:
		return 100;
	case fbxsdk::FbxTime::EMode::eFrames30:
		return 30;
	case fbxsdk::FbxTime::EMode::eFrames24:
		return 24;
	case fbxsdk::FbxTime::EMode::eFrames120:
		return 120;
	case fbxsdk::FbxTime::EMode::eFrames60:
		return 60;
	case fbxsdk::FbxTime::EMode::eFrames72:
		return 72;
	case fbxsdk::FbxTime::EMode::eFrames50:
		return 50;
	default:
		return -1;
	}
}

bool SceneToChunk(FbxScene* fbxScene, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	chunk.allocs = allocs;

	FbxGeometryConverter* conv = new FbxGeometryConverter(g_FbxManager);
	conv->Triangulate(fbxScene, true);
	delete conv;

	uint meshCount = 0, animCount = 0;
	{
		std::vector<FbxNode*> nodeVector;
		nodeVector.push_back(fbxScene->GetRootNode());

		while (nodeVector.size())
		{
			FbxNode* node = nodeVector[nodeVector.size() - 1];
			nodeVector.pop_back();
			for (int i = 0; i < node->GetChildCount(); i++)
				nodeVector.push_back(node->GetChild(i));

			FbxNodeAttribute* attr = node->GetNodeAttribute();
			if (!attr) continue;
			FbxNodeAttribute::EType type = attr->GetAttributeType();

			if (type == FbxNodeAttribute::EType::eMesh)
				meshCount++;
		}

		animCount += (uint)fbxScene->GetSrcObjectCount<FbxAnimStack>();
	}

	chunk.meshCount = 0;
	chunk.meshs = (FBXMeshChunk*)allocs->alloc(sizeof(FBXMeshChunk) * meshCount);
	memset(chunk.meshs, 0, sizeof(FBXMeshChunk) * meshCount);

	chunk.animationCount = 0;
	chunk.animations = (FBXChunk::FBXAnimation*)allocs->alloc(sizeof(FBXChunk::FBXAnimation) * animCount);
	memset(chunk.animations, 0, sizeof(FBXChunk::FBXAnimation) * animCount);

	HierarchyAllocate(fbxScene, chunk, opt, allocs);
	TraversalFBXNode(fbxScene->GetRootNode(), chunk, opt, allocs);
	AnimationToChunk(fbxScene, chunk, allocs);
	BlendShapeToChunk(fbxScene, chunk, allocs);

	return true;
}

void FbxAMatrixToMatrix4x4(const fbxsdk::FbxAMatrix& fbxMatrix, Matrix4x4& matrix)
{
	for (int i = 0; i < 16; i++)
		matrix.dataf[i] = (float)((const double*)fbxMatrix)[i];
}

void HierarchyAllocate(FbxScene* fbxScene, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	puts(fbxScene->GetName());

	int fbxNodeCount = fbxScene->GetNodeCount(), currentNodeCount = 0;
	chunk.hierarchyNodes = (FBXHierarchyNode*)allocs->alloc(sizeof(FBXHierarchyNode) * fbxNodeCount);
	memset(chunk.hierarchyNodes, 0, sizeof(FBXHierarchyNode) * fbxNodeCount);
	struct qNode {
		int depth;
		int hierarchyParentIndex;
		int childIndex;
		FbxNode* fbxNode;
	};
	struct qNodeCompare {
		bool operator()(qNode &lhs, qNode &rhs) {
			if (lhs.depth == rhs.depth)
			{
				if (lhs.hierarchyParentIndex == rhs.hierarchyParentIndex)
					return lhs.childIndex > rhs.childIndex;
				else
					return lhs.hierarchyParentIndex > rhs.hierarchyParentIndex;
			}
			else
				return lhs.depth > rhs.depth;
		};
	};
	std::priority_queue<qNode, std::vector<qNode>, qNodeCompare> q;
	for (int i = 0; i < fbxNodeCount; i++)
		if (fbxScene->GetNode(i) == fbxScene->GetRootNode())
		{
			q.push(qNode{ 0, -1, 0, fbxScene->GetNode(i) });
			break;
		}
			
	while (q.size() > 0)
	{
		qNode currentNode = q.top();
		q.pop();
		for (int i = 0; i < currentNode.fbxNode->GetChildCount(); i++)
			q.push({ currentNode.depth + 1, currentNodeCount, i, currentNode.fbxNode->GetChild(i) });

		const char* name = currentNode.fbxNode->GetName();
		size_t len = strlen(name);
		FBXHierarchyNode& hiNode = chunk.hierarchyNodes[currentNodeCount];

		hiNode.name = (char*)allocs->alloc(sizeof(char) * (len + 1));
		strcpy_s(hiNode.name, len + 1, name);
		hiNode.parentIndex = currentNode.hierarchyParentIndex;
		hiNode.childIndexStart = 0;
		hiNode.childCount = 0;

		if (hiNode.parentIndex >= 0)
		{
			FBXHierarchyNode& parentNode = chunk.hierarchyNodes[hiNode.parentIndex];
			if (parentNode.childCount == 0)
				parentNode.childIndexStart = currentNodeCount;			
			parentNode.childCount++;
		} 

		//const char* skeletonTypes[] = { "Root", "Limb", "Limb Node", "Effector" };
		//auto attr = currentNode.fbxNode->GetNodeAttribute();
		//if (attr && attr->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton)
		//{
		//	FbxSkeleton* skel = (FbxSkeleton*)currentNode.fbxNode->GetNodeAttribute();
		//	printf("skeleton name : %s\n", currentNode.fbxNode->GetName());
		//	printf("type : %s\n", skeletonTypes[skel->GetSkeletonType()]);
		//}

		currentNodeCount++;
	}

	chunk.hierarchyCount = currentNodeCount;
}

bool MeshToChunk(FbxNode* node,  FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);

uint TraversalFBXNode(FbxNode* node, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	uint count = 0;
	FbxNodeAttribute* attr = node->GetNodeAttribute();
	if (attr != nullptr)
	{
		FbxNodeAttribute::EType type = attr->GetAttributeType();

		switch (type)
		{
		case FbxNodeAttribute::EType::eMesh:
			if (MeshToChunk(node, chunk, opt, allocs))
				count++;
			break;
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
		count += TraversalFBXNode(node->GetChild(i), chunk, opt, allocs);

	return count;
}

bool LinkToChunk(FbxNode* node, FBXChunk& wholeChunk, FBXMeshChunk& meshChunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);
bool MeshToChunk(FbxNode* node, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	const char* name = node->GetNameOnly();
	FbxMesh* fbxMesh = node->GetMesh();

	chunk.meshCount++;
	FBXMeshChunk& mesh = chunk.meshs[chunk.meshCount - 1];
	memset(chunk.meshs + (chunk.meshCount - 1), 0, sizeof(FBXMeshChunk));
	ALLOC_AND_STRCPY(mesh.name, name, allocs->alloc);
	mesh.geometry.vertexCount = fbxMesh->GetControlPointsCount();

	// controlpoint to vertex
	FbxVector4* fbxVertices = fbxMesh->GetControlPoints();
	mesh.geometry.vertices = (Vector3f*)allocs->alloc(sizeof(Vector3f) * mesh.geometry.vertexCount);
	Vector3f min = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX), max = Vector3f(FLT_MIN, FLT_MIN, FLT_MIN);
	for (uint i = 0; i < mesh.geometry.vertexCount; i++)
	{
		mesh.geometry.vertices[i].x = static_cast<float>(fbxVertices[i].mData[0]);
		mesh.geometry.vertices[i].y = static_cast<float>(fbxVertices[i].mData[1]);
		mesh.geometry.vertices[i].z = static_cast<float>(fbxVertices[i].mData[2]);

		min = Min(mesh.geometry.vertices[i], min);
		max = Max(mesh.geometry.vertices[i], max);
	}
	mesh.geometry.bound = Bounds((min + max) / 2.0f, Abs(min - max));

	// non-unifoirm polygon to uniform triangle
	bool clockwisedivide = false, submeshExist = false;
	int lastTriangleIndex = 0, polygonSize;
	std::vector<uint> triangles;
	triangles.reserve(fbxMesh->GetPolygonCount() * 3);

	for (int i = 0; i < fbxMesh->GetPolygonCount(); i++)
	{
		// polygon divide as triangle
		polygonSize = fbxMesh->GetPolygonSize(i);

		FALSE_ERROR_MESSAGE_ARGS_RETURN_CODE(
			polygonSize == 3,
			L"fail to divide polygon(%d), because only avaiable polygonsize is 3..",
			false,
			polygonSize
		);

		if (!opt->flipface)
		{
			triangles.push_back(fbxMesh->GetPolygonVertex(i, 0));
			triangles.push_back(fbxMesh->GetPolygonVertex(i, 1));
			triangles.push_back(fbxMesh->GetPolygonVertex(i, 2));
		}
		else
		{
			triangles.push_back(fbxMesh->GetPolygonVertex(i, 0));
			triangles.push_back(fbxMesh->GetPolygonVertex(i, 2));
			triangles.push_back(fbxMesh->GetPolygonVertex(i, 1));
		}
	}

	mesh.geometry.indexCount = (uint)triangles.size();
	mesh.geometry.indices = (uint*)allocs->alloc(sizeof(uint) * mesh.geometry.indexCount);
	memcpy(mesh.geometry.indices, triangles.data(), sizeof(uint) * mesh.geometry.indexCount);

#pragma region TODO:: 서브메시 할당!

	mesh.submesh.submeshCount = 0;
	mesh.submesh.submeshs = nullptr;

#pragma endregion

	// normal
	if (fbxMesh->GetElementNormalCount() > 0)
	{
		FALSE_WARN_MESSAGE_ARGS(
			fbxMesh->GetElementNormalCount() == 1,
			L"Mesh(from %s) has %d normal array, just hard-coded for first normal array..",
			fbxMesh->GetScene()->GetName(), fbxMesh->GetElementNormalCount()
		);

		FbxGeometryElementNormal* fbxNormal = fbxMesh->GetElementNormal(0);
		mesh.geometry.normals = (Vector3f*)allocs->alloc(sizeof(Vector3f) * mesh.geometry.vertexCount);

		if (fbxNormal->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
		{
			if (fbxNormal->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (uint i = 0; i < mesh.geometry.vertexCount; i++)
				{
					mesh.geometry.normals[i].x = static_cast<float>(fbxNormal->GetDirectArray()[i].mData[0]);
					mesh.geometry.normals[i].y = static_cast<float>(fbxNormal->GetDirectArray()[i].mData[1]);
					mesh.geometry.normals[i].z = static_cast<float>(fbxNormal->GetDirectArray()[i].mData[2]);
				}
			}
			else
				WARN_MESSAGE("fail to read normal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else if (fbxNormal->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
		{
			if (fbxNormal->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (uint i = 0; i < mesh.geometry.indexCount; i++)
				{
					int vertexIndex = mesh.geometry.indices[i];
					mesh.geometry.normals[vertexIndex].x = static_cast<float>(fbxNormal->GetDirectArray()[i].mData[0]);
					mesh.geometry.normals[vertexIndex].y = static_cast<float>(fbxNormal->GetDirectArray()[i].mData[1]);
					mesh.geometry.normals[vertexIndex].z = static_cast<float>(fbxNormal->GetDirectArray()[i].mData[2]);
				}
			}
			else if (fbxNormal->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
			{
				for (uint i = 0; i < mesh.geometry.indexCount; i++)
				{
					int vertexIndex = fbxNormal->GetIndexArray()[i];
					mesh.geometry.normals[vertexIndex].x = static_cast<float>(fbxNormal->GetDirectArray()[vertexIndex].mData[0]);
					mesh.geometry.normals[vertexIndex].y = static_cast<float>(fbxNormal->GetDirectArray()[vertexIndex].mData[1]);
					mesh.geometry.normals[vertexIndex].z = static_cast<float>(fbxNormal->GetDirectArray()[vertexIndex].mData[2]);
				}
			}
			else
				WARN_MESSAGE("fail to read normal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else
			WARN_MESSAGE("fail to read normal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
	}


	// tangent
	if (fbxMesh->GetElementTangentCount() > 0)
	{
		FALSE_WARN_MESSAGE_ARGS(
			fbxMesh->GetElementTangentCount() == 1,
			L"Mesh(from %s) has %d tangent array, just hard-coded for first tangent array..",
			fbxMesh->GetScene()->GetName(), fbxMesh->GetElementTangentCount()
		);

		FbxGeometryElementTangent* fbxTangent = fbxMesh->GetElementTangent(0);
		mesh.geometry.tangents = (Vector3f*)allocs->alloc(sizeof(Vector3f) * mesh.geometry.vertexCount);

		if (fbxTangent->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
		{
			if (fbxTangent->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (uint i = 0; i < mesh.geometry.vertexCount; i++)
				{
					mesh.geometry.tangents[i].x = static_cast<float>(fbxTangent->GetDirectArray()[i].mData[0]);
					mesh.geometry.tangents[i].y = static_cast<float>(fbxTangent->GetDirectArray()[i].mData[1]);
					mesh.geometry.tangents[i].z = static_cast<float>(fbxTangent->GetDirectArray()[i].mData[2]);
				}
			}
			else
				WARN_MESSAGE("fail to read tangent, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else if (fbxTangent->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
		{
			if (fbxTangent->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (uint i = 0; i < mesh.geometry.indexCount; i++)
				{
					int vertexIndex = mesh.geometry.indices[i];
					mesh.geometry.tangents[vertexIndex].x = static_cast<float>(fbxTangent->GetDirectArray()[i].mData[0]);
					mesh.geometry.tangents[vertexIndex].y = static_cast<float>(fbxTangent->GetDirectArray()[i].mData[1]);
					mesh.geometry.tangents[vertexIndex].z = static_cast<float>(fbxTangent->GetDirectArray()[i].mData[2]);
				}
			}
			else if (fbxTangent->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
			{
				for (uint i = 0; i < mesh.geometry.indexCount; i++)
				{
					int vertexIndex = fbxTangent->GetIndexArray()[i];
					mesh.geometry.tangents[vertexIndex].x = static_cast<float>(fbxTangent->GetDirectArray()[vertexIndex].mData[0]);
					mesh.geometry.tangents[vertexIndex].y = static_cast<float>(fbxTangent->GetDirectArray()[vertexIndex].mData[1]);
					mesh.geometry.tangents[vertexIndex].z = static_cast<float>(fbxTangent->GetDirectArray()[vertexIndex].mData[2]);
				}
			}
			else
				WARN_MESSAGE("fail to read tangent, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else
			WARN_MESSAGE("fail to read tangent, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
	}

	// binormal
	if (fbxMesh->GetElementBinormalCount() > 0)
	{
		FALSE_WARN_MESSAGE_ARGS(
			fbxMesh->GetElementBinormalCount() == 1,
			L"Mesh(from %s) has %d binormal array, just hard-coded for first binormal array..",
			fbxMesh->GetScene()->GetName(), fbxMesh->GetElementBinormalCount()
		);

		FbxGeometryElementBinormal* fbxBinormal = fbxMesh->GetElementBinormal(0);
		mesh.geometry.binormals = (Vector3f*)allocs->alloc(sizeof(Vector3f) * mesh.geometry.vertexCount);

		if (fbxBinormal->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
		{
			if (fbxBinormal->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (uint i = 0; i < mesh.geometry.vertexCount; i++)
				{
					mesh.geometry.binormals[i].x = static_cast<float>(fbxBinormal->GetDirectArray()[i].mData[0]);
					mesh.geometry.binormals[i].y = static_cast<float>(fbxBinormal->GetDirectArray()[i].mData[1]);
					mesh.geometry.binormals[i].z = static_cast<float>(fbxBinormal->GetDirectArray()[i].mData[2]);
				}
			}
			else
				WARN_MESSAGE("fail to read binormal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else if (fbxBinormal->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
		{
			if (fbxBinormal->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (uint i = 0; i < mesh.geometry.indexCount; i++)
				{
					int vertexIndex = mesh.geometry.indices[i];
					mesh.geometry.binormals[vertexIndex].x = static_cast<float>(fbxBinormal->GetDirectArray()[i].mData[0]);
					mesh.geometry.binormals[vertexIndex].y = static_cast<float>(fbxBinormal->GetDirectArray()[i].mData[1]);
					mesh.geometry.binormals[vertexIndex].z = static_cast<float>(fbxBinormal->GetDirectArray()[i].mData[2]);
				}
			}
			else if (fbxBinormal->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
			{
				for (uint i = 0; i < mesh.geometry.indexCount; i++)
				{
					int vertexIndex = fbxBinormal->GetIndexArray()[i];
					mesh.geometry.binormals[vertexIndex].x = static_cast<float>(fbxBinormal->GetDirectArray()[vertexIndex].mData[0]);
					mesh.geometry.binormals[vertexIndex].y = static_cast<float>(fbxBinormal->GetDirectArray()[vertexIndex].mData[1]);
					mesh.geometry.binormals[vertexIndex].z = static_cast<float>(fbxBinormal->GetDirectArray()[vertexIndex].mData[2]);
				}
			}
			else					
				WARN_MESSAGE("fail to read binormal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else
			WARN_MESSAGE("fail to read binormal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..");
	}

	// uv
	mesh.geometry.uvSlotCount = fbxMesh->GetElementUVCount();
	mesh.geometry.uvSlots = (Vector2f**)allocs->alloc(sizeof(Vector2f*) * mesh.geometry.uvSlotCount);
	memset(mesh.geometry.uvSlots, 0, sizeof(sizeof(Vector2f*) * mesh.geometry.uvSlotCount));

	for (int uvArrayIdx = 0; uvArrayIdx < fbxMesh->GetElementUVCount(); uvArrayIdx++)
	{
		FbxGeometryElementUV* fbxUV = fbxMesh->GetElementUV(uvArrayIdx);
		Vector2f* uvs = mesh.geometry.uvSlots[uvArrayIdx] = (Vector2f*)allocs->alloc(sizeof(Vector2f) * mesh.geometry.vertexCount);
		memset(uvs, 0, sizeof(sizeof(Vector2f) * mesh.geometry.vertexCount));

		int polygonCount = fbxMesh->GetPolygonCount();

		if (fbxUV->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
		{
			if (fbxUV->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect)
			{
				for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
				{
					for (int itemIndex = 0; itemIndex < fbxMesh->GetPolygonSize(polyIndex); itemIndex++)
					{
						int controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, itemIndex);
						
						uvs[controlPointIndex].x = static_cast<float>(fbxUV->GetDirectArray()[controlPointIndex].mData[0]);
						uvs[controlPointIndex].y = static_cast<float>(fbxUV->GetDirectArray()[controlPointIndex].mData[1]);

						if (opt->flipU)
							uvs[controlPointIndex].x = 1.f - uvs[controlPointIndex].x;
						if (opt->flipV)
							uvs[controlPointIndex].y = 1.f - uvs[controlPointIndex].y;
					}
				}					
			}
			else if (fbxUV->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
			{
				for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
				{
					for (int itemIndex = 0; itemIndex < fbxMesh->GetPolygonSize(polyIndex); itemIndex++)
					{
						int controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, itemIndex);
						int uvIndex = fbxUV->GetIndexArray()[controlPointIndex];
						uvs[controlPointIndex].x = static_cast<float>(fbxUV->GetDirectArray()[uvIndex].mData[0]);
						uvs[controlPointIndex].y = static_cast<float>(fbxUV->GetDirectArray()[uvIndex].mData[1]);

						if (opt->flipU)
							uvs[controlPointIndex].x = 1.f - uvs[controlPointIndex].x;
						if (opt->flipV)
							uvs[controlPointIndex].y = 1.f - uvs[controlPointIndex].y;
					}
				}
			}
			else
				WARN_MESSAGE("fail to read uv, only eByControlPoint/eDrict+eIndexToDirect, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else if (fbxUV->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
		{
			if (fbxUV->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect ||
				fbxUV->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
			{
				for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
				{
					for (int itemIndex = 0; itemIndex < fbxMesh->GetPolygonSize(polyIndex); itemIndex++)
					{
						int controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, itemIndex);
						int textureUVIndex = fbxMesh->GetTextureUVIndex(polyIndex, itemIndex);
						uvs[controlPointIndex].x = static_cast<float>(fbxUV->GetDirectArray()[textureUVIndex].mData[0]);
						uvs[controlPointIndex].y = static_cast<float>(fbxUV->GetDirectArray()[textureUVIndex].mData[1]);

						if (opt->flipU)
							uvs[controlPointIndex].x = 1.f - uvs[controlPointIndex].x;
						if (opt->flipV)
							uvs[controlPointIndex].y = 1.f - uvs[controlPointIndex].y;
					}
				}
			}
			else
				WARN_MESSAGE("fail to read uv, only eByControlPoint/eDrict+eIndexToDirect, eByPolygonVertex/eDriect+eIndexToDriect..");
		}
		else
			WARN_MESSAGE("fail to read uv, only eByControlPoint/eDrict+eIndexToDirect, eByPolygonVertex/eDriect+eIndexToDriect..");
	}

	LinkToChunk(node, chunk, mesh, opt, allocs);

	return true;
}

const char* g_ClusterModes[] = { "Normalize", "Additive", "Total1" };

bool LinkToChunk(FbxNode* node, FBXChunk& wholeChunk, FBXMeshChunk& meshChunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	const char* name = node->GetName();
	FbxMesh* fbxMesh = node->GetMesh();
	FbxAMatrix geometricTransform =
		FbxAMatrix(
			node->GetGeometricTranslation(FbxNode::eSourcePivot),
			node->GetGeometricRotation(FbxNode::eSourcePivot),
			node->GetGeometricScaling(FbxNode::eSourcePivot)
		);

	int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	if (skinCount >= 0)
	{
		FALSE_WARN_MESSAGE_ARGS(skinCount == 1, L"Mesh(%s) has many skin inform, selected as %dst skin..", name, opt->fbxSkinIndex);
		FbxSkin* skin = (FbxSkin*)fbxMesh->GetDeformer(opt->fbxSkinIndex, FbxDeformer::eSkin);

		struct Bone {
			int index; float weight;
		};
		std::vector<Bone>* countPerVertices = (std::vector<Bone>*)alloca(meshChunk.geometry.vertexCount * sizeof(std::vector<Bone>));
		for (uint cvvi = 0; cvvi < meshChunk.geometry.vertexCount; cvvi++)
			new (countPerVertices + cvvi) std::vector<Bone>();

		int clusterCount = skin->GetClusterCount();
		for (int clusterIdx = 0; clusterIdx < clusterCount; clusterIdx++)
		{
			fbxsdk::FbxCluster* cluster = skin->GetCluster(clusterIdx);
			if (cluster->GetLinkMode() != fbxsdk::FbxCluster::ELinkMode::eNormalize)
			{
				WARN_MESSAGE_ARGS(L"link mode is not normalized, %s..", g_ClusterModes[cluster->GetLinkMode()]);
				continue;
			}

			int boneRefVertexCount = cluster->GetControlPointIndicesCount();
			int* vertexIndices = cluster->GetControlPointIndices();
			double* boneWeights = cluster->GetControlPointWeights();

			int hierarchyIndex = -1;
			for (uint i = 0; i < wholeChunk.hierarchyCount; i++)
				if (strcmp(wholeChunk.hierarchyNodes[i].name, cluster->GetLink()->GetName()) == 0)
				{
					hierarchyIndex = i;
					break;
				}
			FALSE_ERROR_MESSAGE_CONTINUE_ARGS(
				hierarchyIndex >= 0, 
				L"fail to find same hierarchy names..(%s)",
				cluster->GetLink()->GetName()
			);

			FbxAMatrix transformMatrix, transformLinkMatrix, bindPoseMatrix;
			cluster->GetTransformMatrix(transformMatrix);
			cluster->GetTransformLinkMatrix(transformLinkMatrix);
			FbxAMatrixToMatrix4x4(
				transformLinkMatrix.Inverse() * transformMatrix * geometricTransform,
				wholeChunk.hierarchyNodes[hierarchyIndex].inverseGlobalTransformMatrix
			);

			for (int i = 0; i < boneRefVertexCount; i++)
				countPerVertices[vertexIndices[i]].push_back({ hierarchyIndex, (float)boneWeights[i] });
		}

		meshChunk.geometry.boneIndices = (Vector4i*)allocs->alloc(sizeof(Vector4i) * meshChunk.geometry.vertexCount);
		memset(meshChunk.geometry.boneIndices, 0, sizeof(Vector4i) * meshChunk.geometry.vertexCount);
		meshChunk.geometry.boneWeights = (Vector4f*)allocs->alloc(sizeof(Vector4f) * meshChunk.geometry.vertexCount);
		memset(meshChunk.geometry.boneWeights, 0, sizeof(Vector4f) * meshChunk.geometry.vertexCount);

		for (uint vi = 0; vi < meshChunk.geometry.vertexCount; vi++)
		{
			std::sort(
				countPerVertices[vi].begin(),
				countPerVertices[vi].end(),
				[](Bone bi0, Bone bi1) {
					return (bi0.weight - bi1.weight) > 0 ? true : false;
				}
			);

			float weightSum = 0.f;
			for (int bi = 0; bi < 4 && bi < countPerVertices[vi].size(); bi++)
			{
				meshChunk.geometry.boneIndices[vi][bi] = countPerVertices[vi][bi].index;
				meshChunk.geometry.boneWeights[vi][bi] = countPerVertices[vi][bi].weight;
				weightSum += countPerVertices[vi][bi].weight;
			}

			// approximate 4 bone weight sum is 1,(L1 normalize)
			if (weightSum != 1.f)
			{
				FALSE_WARN_MESSAGE_ARGS(
					fabs(weightSum - 1.f) < 0.01, 
					L"vertex weight sum has error.. (%.2f)", 
					fabs(weightSum - 1.f)
				);

				meshChunk.geometry.boneWeights[vi] /= weightSum;
			}
		}

		return true;
	}
	else
	{
		meshChunk.geometry.boneIndices = nullptr;
		meshChunk.geometry.boneWeights = nullptr;
		return false;
	}
}

// https://github.com/lang1991/FBXExporter/blob/master/FBXExporter/FBXExporter.cpp
bool AnimationToChunk(FbxScene* scene, FBXChunk& chunk, const Allocaters* allocs)
{
	FbxNode* rootNode = scene->GetRootNode();
	FbxGlobalSettings& globalSettings = scene->GetGlobalSettings();
	fbxsdk::FbxTime::EMode timeMode = globalSettings.GetTimeMode();

	int prevAnimCount = chunk.animationCount, 
		newAnimCount = scene->GetSrcObjectCount<FbxAnimStack>();
	chunk.animationCount += (uint)newAnimCount;

	//uint clusterCnt = skin->GetClusterCount();
	FbxAMatrix geometricTransform =
		FbxAMatrix(
			rootNode->GetGeometricTranslation(FbxNode::eSourcePivot),
			rootNode->GetGeometricRotation(FbxNode::eSourcePivot),
			rootNode->GetGeometricScaling(FbxNode::eSourcePivot)
		);

	FbxNode** nodes = (FbxNode**)alloca(sizeof(FbxNode*) * chunk.hierarchyCount);
	for (uint hi = 0; hi < chunk.hierarchyCount; hi++)
		nodes[hi] = scene->FindNodeByName(chunk.hierarchyNodes[hi].name);

	for (int ai = 0; ai < newAnimCount; ai++)
	{
		FbxAnimStack* currAnimStack = scene->GetSrcObject<FbxAnimStack>(ai);
		FBXChunk::FBXAnimation& anim = chunk.animations[ai + prevAnimCount];

		FbxString animStackName = currAnimStack->GetName();
		FbxTakeInfo* takeInfo = scene->GetTakeInfo(animStackName);
		FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

		const char* animationName = animStackName.Buffer();
		ALLOC_AND_STRCPY(anim.animationName, animationName, allocs->alloc);

		anim.frameKeyCount = static_cast<uint>(end.GetFrameCount(timeMode) - start.GetFrameCount(timeMode)) + 1;
		anim.fpsCount = GetFPS(timeMode);
		anim.globalAffineTransforms = (Matrix4x4*)allocs->alloc(sizeof(Matrix4x4) * chunk.hierarchyCount * anim.frameKeyCount);
		memset(anim.globalAffineTransforms, 0, sizeof(Matrix4x4) * chunk.hierarchyCount * anim.frameKeyCount);

		FbxLongLong frameStartIndex = start.GetFrameCount(timeMode);
		for (FbxLongLong frameIndex = frameStartIndex; frameIndex <= end.GetFrameCount(timeMode); ++frameIndex)
		{
			FbxTime currTime;
			currTime.SetFrame(frameIndex, timeMode);

			for (uint hi = 0; hi < chunk.hierarchyCount; hi++)
			{
				FbxAMatrix matrix = geometricTransform * nodes[hi]->EvaluateGlobalTransform(currTime);
				FbxAMatrixToMatrix4x4(
					matrix,
					anim.globalAffineTransforms[((frameIndex - frameStartIndex) * chunk.hierarchyCount + hi)]
				);
			}
		}
	}

	return true;
}

bool BlendShapeToChunk(FbxScene* fbxScene, FBXChunk& chunk, const Allocaters* allocs)
{
	return true;
}
