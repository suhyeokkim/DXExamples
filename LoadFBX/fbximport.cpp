#include <fbxsdk.h>
#include "fbximport.h"

#if defined(_DEBUG) || defined(DEBUG)
	#pragma comment(lib, "zlib-mt.lib")
	#pragma comment(lib, "libxml2-mt.lib")
	#pragma comment(lib, "libfbxsdk-mt.lib")
#else
	#pragma comment(lib, "libfbxsdk.lib")
#endif 

FbxManager* g_FbxManager = nullptr;

void InitializeFBX()
{
	g_FbxManager = FbxManager::Create();
	
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

bool ImportFBX(const wchar_t* fileDirectory, void** p)
{
	FbxImporter* fbxImporter = FbxImporter::Create(g_FbxManager, "");


	return false;
}

