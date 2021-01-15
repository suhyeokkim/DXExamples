#include <fbxsdk.h>
#include <vector>
#include <queue>
#include <algorithm>

using namespace fbxsdk;

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "triangulateFBX <input FBX Path> <output FBX Path>");
		return -1;
	}

	printf("triangulate input file(%s) to path(%s)\n", argv[1], argv[2]);

	char* importFilePath = argv[1];
	FbxManager* fbxManager = FbxManager::Create();

	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "fbxImporter");
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "");

	if (!fbxImporter->Initialize(importFilePath, -1, fbxManager->GetIOSettings()))
	{
		fprintf(stderr, "triangulateFBX <input FBX Path> <output FBX Path>");
		return -1;
	}

	if (!fbxImporter->Import(fbxScene))
	{
		fprintf(stderr, "triangulateFBX <input FBX Path> <output FBX Path>");
		return -1;
	}

	{
		FbxGeometryConverter* conv = new FbxGeometryConverter(fbxManager);
		if (!conv->Triangulate(fbxScene, true))
		{
			fprintf(stderr, "triangulateFBX :: fail to triangulate..");
			return -1;
		}
		delete conv;
	}

	char* exportFilePath = argv[2];
	FbxExporter* fbxExporter = FbxExporter::Create(fbxManager, "fbxExporter");
	if (!fbxExporter->Initialize(exportFilePath, -1, fbxManager->GetIOSettings()))
	{
		fprintf(stderr, "triangulateFBX <input FBX Path> <output FBX Path>");
		return -1;
	}
	if (!fbxExporter->Export(fbxScene))
	{
		fprintf(stderr, "triangulateFBX <input FBX Path> <output FBX Path>");
		return -1;
	}

	fbxManager->Destroy();

	printf("success triangulate!");

	return 0;
}
