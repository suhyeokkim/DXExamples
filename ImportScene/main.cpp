#include <iostream>
#include <chrono>
#include <fbxsdk.h>

#include "fbximport.h"

void PrintFBXChunk(FILE* fp, FBXChunk* chunk)
{
	uint i, j, k;
	for (i = 0; i < chunk->meshCount; i++)
	{
		FBXMeshChunk& m = chunk->meshs[i];
		fprintf(fp, "meshname:%s, vertexcount:%d, indexcount:%d\n", m.name, m.geometry.vertexCount, m.geometry.indexCount);

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

	for (i = 0; i < chunk->hierarchyCount; i++)
	{
		auto& node = chunk->hierarchyNodes[i];

		fprintf(
			fp, "name: %s, pidx: %d, cidx: %d, ccount: %d\n", 
			node.name, node.parentIndex, node.childIndexStart, node.childCount
		);

		auto& gm = node.inverseGlobalTransformMatrix;

		fprintf(fp, "inv. global affine transform\n");
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[0], gm[4], gm[8], gm[12]);
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[1], gm[5], gm[9], gm[13]);
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[2], gm[6], gm[10], gm[14]);
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[3], gm[7], gm[11], gm[15]);
		fputs("\n", fp);
	}

	for (i = 0; i < chunk->animationCount; i++)
	{
		for (j = 0; j < chunk->animations[i].frameKeyCount; j++)
		{
			auto &gm = chunk->animations[i].globalAffineTransforms[j];

			fprintf(fp, "frame index : %d\n", i);

			fprintf(fp, "global affine transform\n");
			fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[0], gm[4], gm[8], gm[12]);
			fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[1], gm[5], gm[9], gm[13]);
			fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[2], gm[6], gm[10], gm[14]);
			fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", gm[3], gm[7], gm[11], gm[15]);
			fputs("\n", fp);
		}
	}
	
}

void PrintTRSTest(FILE *fp)
{
	fputs("\n", fp);
	//*/
	Vector3f pos[] = { Vector3f(-1,-1,-1), Vector3f(1,1,1), Vector3f(-1,1,-1), Vector3f(1,-1,-1) };
	/*/
	Vector3f pos[] = { Vector3f::Zero(), Vector3f::Zero(), Vector3f::Zero(), Vector3f::Zero() };
	//*/
	Quaternion qs[] = {
		AngleAxis(45.0f, pos[0].normalized()),
		AngleAxis(22.5f, pos[1].normalized()),
		AngleAxis(77.5f, pos[2].normalized()),
		AngleAxis(90.0f, pos[3].normalized()),
	};
	struct Euler { Vector3f angle; EulerAngleOrder order; } eulers[] = 
		{
			{ Vector3f(90.f, -90.f, 45.f) , EulerAngleOrder::OrderZXY }, 
			{ Vector3f(0.0f, 0.0f, 90.0f) , EulerAngleOrder::OrderZXY },
			{ Vector3f(0.0f, 90.0f, 0.0f) , EulerAngleOrder::OrderZXY },
			{ Vector3f(90.0f, 0.0f, 00.0f), EulerAngleOrder::OrderZXY },
		};
	//*/
	Vector3f scl[] = { Vector3f::One(), Vector3f(0.5f, 1.f, 1.f), Vector3f(1.f, 0.5f, 1.f), Vector3f(1.f, 1.f, 0.5f) };
	/*/
	Vector3f scl[] = { Vector3f::One(), Vector3f::One(), Vector3f::One(), Vector3f::One() };
	//*/
	Vector3f pps[] = { Vector3f::One(), Vector3f::One(), Vector3f::One(), Vector3f::One() };

	for (int i = 0; i < 4; i++, fputs("\n", fp))
	{
		const Vector3f& p = pos[i], &s = scl[i];
		//*/
		const Quaternion& q = qs[i];
		fprintf(fp, "T: (%.2f, %.2f, %.2f), R: (%.2f, %.2f, %.2f, %.2f), S: (%.2f, %.2f, %.2f)\n", p.x, p.y, p.z, q.x, q.y, q.z, q.w, s.x, s.y, s.z);
		Matrix4x4 m = Matrix4x4::FromTRS(p, q, s);
		/*/
		const Euler& e = eulers[i];
		fprintf(fp, "T: (%.2f, %.2f, %.2f), R: (%.2f, %.2f, %.2f), S: (%.2f, %.2f, %.2f)\n", p.x, p.y, p.z, e.angle.x, e.angle.y, e.angle.z, s.x, s.y, s.z);
		Matrix4x4 m = Matrix4x4::FromTRS(p, e.angle, e.order, s);
		//*/
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", m[0], m[4], m[8], m[12]);
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", m[1], m[5], m[9], m[13]);
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", m[2], m[6], m[10], m[14]);
		fprintf(fp, "%.2f, %.2f, %.2f, %.2f\n", m[3], m[7], m[11], m[15]);

		Vector3f& pp = pps[i], tp = m.TransformPoint(pp);
		fprintf(fp, "transform:: (%.2f, %.2f, %.2f) -> (%.2f, %.2f, %.2f)\n", pp.x, pp.y, pp.z, tp.x, tp.y, tp.z);

		fbxsdk::FbxAMatrix fm;
		fm.SetTQS(fbxsdk::FbxVector4(p.x, p.y, p.z), fbxsdk::FbxQuaternion(q.x, q.y, q.z, q.w), fbxsdk::FbxVector4(s.x, s.y, s.z));
		double* d = fm;

		fprintf(fp, "fbx matrix\n");
		fprintf(fp, "%.2lf, %.2lf, %.2lf, %.2lf\n", d[0], d[4], d[8], d[12]);
		fprintf(fp, "%.2lf, %.2lf, %.2lf, %.2lf\n", d[1], d[5], d[9], d[13]);
		fprintf(fp, "%.2lf, %.2lf, %.2lf, %.2lf\n", d[2], d[6], d[10], d[14]);
		fprintf(fp, "%.2lf, %.2lf, %.2lf, %.2lf\n", d[3], d[7], d[11], d[15]);

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
