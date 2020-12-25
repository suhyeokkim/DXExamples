#include <fbxsdk.h>
#include <vector>

#include "fbximport.h"

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

Allocaters g_GlobalAllocater = { malloc, realloc, free };
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

	importStatus = SceneToChunk(fbxScene, chunk, opt? opt: &dfltopt, allocs != nullptr ? allocs : &g_GlobalAllocater);
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

	importStatus = SceneToChunk(fbxScene, chunk, opt? opt: &dfltopt, allocs != nullptr ? allocs : &g_GlobalAllocater);
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

		importStatus = SceneToChunk(fbxScene, *(chunk + i), opt? opt + i: &dfltopt, allocs != nullptr ? allocs : &g_GlobalAllocater);
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

		importStatus = SceneToChunk(fbxScene, *(chunk + i), opt ? opt + i : &dfltopt, allocs != nullptr ? allocs : &g_GlobalAllocater);
		FALSE_ERROR_MESSAGE_CONTINUE(importStatus, L"fail to copy to chunk..");

		if (fbxImporter) fbxImporter->Destroy();
		if (fbxScene) fbxScene->Destroy();

		validChunkCount++;
	}

	return validChunkCount;
}

uint TraversalFBXNode(FbxNode* node, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);
bool SkeletonToChunk(FbxScene* fbxScene, FBXChunk& chunk, const Allocaters* allocs);
bool BlendShapeToChunk(FbxScene* fbxScene, FBXChunk& chunk, const Allocaters* allocs);

bool SceneToChunk(FbxScene* fbxScene, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	chunk.allocs = allocs;
	FbxGeometryConverter* conv = new FbxGeometryConverter(g_FbxManager);
	conv->Triangulate(fbxScene, true);

	TraversalFBXNode(fbxScene->GetRootNode(), chunk, opt, allocs);
	SkeletonToChunk(fbxScene, chunk, allocs);
	BlendShapeToChunk(fbxScene, chunk, allocs);

	delete conv;

	return true;
}

bool MeshToChunk(const char *name, FbxMesh* fbxMesh, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs);
bool SkeletonToChunk(FbxNode* fbxNode, FBXChunk& chunk, const Allocaters* allocs);

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
			if (MeshToChunk(node->GetNameOnly(), node->GetMesh(), chunk, opt, allocs))
				count++;
			break;
		case FbxNodeAttribute::EType::eSkeleton:
			if (SkeletonToChunk(node, chunk, allocs))
				count++;
			break;
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
		count += TraversalFBXNode(node->GetChild(i), chunk, opt, allocs);

	return count;
}

bool MeshToChunk(const char* name, FbxMesh* fbxMesh, FBXChunk& chunk, const FBXLoadOptionChunk* opt, const Allocaters* allocs)
{
	chunk.meshCount++;
	if (chunk.meshs == nullptr)
		chunk.meshs = (FBXMeshChunk*)allocs->alloc(sizeof(FBXMeshChunk));
	else
		chunk.meshs = (FBXMeshChunk*)allocs->realloc(chunk.meshs, sizeof(FBXMeshChunk) * chunk.meshCount);

	FBXMeshChunk& mesh = chunk.meshs[chunk.meshCount - 1];
	memset(chunk.meshs + (chunk.meshCount - 1), 0, sizeof(FBXMeshChunk));
	size_t nameLen = strlen(name);
	mesh.name = (char*)allocs->alloc(sizeof(char) * (nameLen+1));
	strcpy_s(mesh.name, sizeof(char) * (nameLen + 1), name);
	mesh.geometry.vertexCount = fbxMesh->GetControlPointsCount();

	// controlpoint to vertex
	FbxVector4* fbxVertices = fbxMesh->GetControlPoints();
	mesh.geometry.vertices = (Vector4f*)allocs->alloc(sizeof(Vector4f) * mesh.geometry.vertexCount);
	for (uint i = 0; i < mesh.geometry.vertexCount; i++)
	{
		mesh.geometry.vertices[i].x = static_cast<float>(fbxVertices[i].mData[0]);
		mesh.geometry.vertices[i].y = static_cast<float>(fbxVertices[i].mData[1]);
		mesh.geometry.vertices[i].z = static_cast<float>(fbxVertices[i].mData[2]);
		mesh.geometry.vertices[i].w = 1.f;
	}

	// non-unifoirm polygon to uniform triangle
	bool clockwisedivide = false, submeshExist = false;
	int lastTriangleIndex = 0, polygonSize, indexCount;
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

	// TODO:: determine submesh
	{
		mesh.submesh.submeshCount = fbxMesh->GetElementPolygonGroupCount();
		mesh.submesh.submeshs = (FBXMeshChunk::FBXSubmesh::Submesh*)allocs->alloc(sizeof(FBXMeshChunk::FBXSubmesh::Submesh) * mesh.submesh.submeshCount);
	}
	if (!submeshExist)
	{
		mesh.submesh.submeshCount = 1;
		mesh.submesh.submeshs = (FBXMeshChunk::FBXSubmesh::Submesh*)allocs->alloc(sizeof(FBXMeshChunk::FBXSubmesh::Submesh) * mesh.submesh.submeshCount);
		mesh.submesh.submeshs[0].indexStart = 0;
		mesh.submesh.submeshs[0].materialRef = 0;
		mesh.submesh.submeshs[0].indexCount = static_cast<int>(triangles.size());
	}

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
				ERROR_MESSAGE_GOTO("fail to read normal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
				ERROR_MESSAGE_GOTO("fail to read normal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
		}
		else
			ERROR_MESSAGE_GOTO("fail to read normal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
				ERROR_MESSAGE_GOTO("fail to read tangent, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
					ERROR_MESSAGE_GOTO("fail to read tangent, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
		}
		else
			ERROR_MESSAGE_GOTO("fail to read tangent, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
				ERROR_MESSAGE_GOTO("fail to read binormal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
				ERROR_MESSAGE_GOTO("fail to read binormal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
		}
		else
			ERROR_MESSAGE_GOTO("fail to read binormal, only eByControlPoint/eDrict, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
				ERROR_MESSAGE_GOTO("fail to read uv, only eByControlPoint/eDrict+eIndexToDirect, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
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
				ERROR_MESSAGE_GOTO("fail to read uv, only eByControlPoint/eDrict+eIndexToDirect, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
		}
		else
			ERROR_MESSAGE_GOTO("fail to read uv, only eByControlPoint/eDrict+eIndexToDirect, eByPolygonVertex/eDriect+eIndexToDriect..", MESH_IMPORT_FAIL);
	}

	return true;

MESH_IMPORT_FAIL:

	return false;
}

bool SkeletonToChunk(FbxNode* fbxNode, FBXChunk& chunk, const Allocaters* allocs)
{
	return true;
}

bool SkeletonToChunk(FbxScene* fbxScene, FBXChunk& chunk, const Allocaters* allocs)
{
	return true;
}

bool BlendShapeToChunk(FbxScene* fbxScene, FBXChunk& chunk, const Allocaters* allocs)
{
	return true;
}
