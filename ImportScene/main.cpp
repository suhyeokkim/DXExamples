#include <iostream>
#include <chrono>

#include "fbximport.h"

void PrintFBXChunk(FILE* fp, FBXChunk* chunk)
{
	for (uint i = 0; i < chunk->meshCount; i++)
	{
		FBXMeshChunk& m = chunk->meshs[i];
		fprintf(fp, "meshname:%s, vertexcount:%d, indexcount:%d\n", m.name, m.geometry.vertexCount, m.geometry.indexCount);

		uint j, k;
		Vector3f* v3;
		Vector2f* v2;
		for (j = 0; j < m.geometry.indexCount; j++)
			fprintf(fp, "idx%d: %d\n", j, m.geometry.indices[j]);

		for (j = 0; j < m.geometry.vertexCount && (v3 = m.geometry.vertices + j); j++)
			fprintf(fp, "pos%d: (%.2f, %.2f, %.2f)\n", j, v3->x, v3->y, v3->z);
		for (j = 0; j < m.geometry.vertexCount && (v3 = m.geometry.tangents + j); j++)
			fprintf(fp, "tan%d: (%.2f, %.2f, %.2f)\n", j, v3->x, v3->y, v3->z);
		for (j = 0; j < m.geometry.vertexCount && (v3 = m.geometry.normals + j); j++)
			fprintf(fp, "nom%d: (%.2f, %.2f, %.2f)\n", j, v3->x, v3->y, v3->z);
		for (j = 0; j < m.geometry.uvSlotCount; j++)
			for (k = 0; k < m.geometry.vertexCount && (v2 = m.geometry.uvSlots[j] + k); k++)
				fprintf(fp, "uv%d-%d: (%.2f, %.2f)\n", j, k, v2->x, v2->y);
	}
}

int main(int argc, char** argv)
{
#if defined (DEBUG) | (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	InitializeFBX();
	{
		FBXChunk* fbxArray = (FBXChunk*)alloca(sizeof(FBXChunk) * (argc - 1));
		for (int i = 0; i < argc - 1; i++)
			new (fbxArray + i) FBXChunk();

		auto start = std::chrono::high_resolution_clock::now();
		uint loadCount = ImportFBX(argc - 1, argv + 1, fbxArray);
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		printf("elapsed time: %lf\n", (double)microseconds / 1000000.0);

		if (loadCount == argc - 1)
			printf("successfully load(%d)!\n", loadCount);
		else if (loadCount > 0)
			printf("partially load(%d/%d).\n", loadCount, argc-1);
		else
			printf("fail to all file(%d) loading..\n", argc-1);

		FILE* fp;
		fopen_s(&fp, "log.log", "wt");

		for (int i = 0; i < argc - 1; i++)
		{
			PrintFBXChunk(fp, fbxArray + i);
			fbxArray[i].~FBXChunk();
		}

		fclose(fp);
	}
	DestroyFBX();


	return 0;
}
