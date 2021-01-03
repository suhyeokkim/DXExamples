#include <DirectXTex.h>
#include <vector>
#include <d3d11_4.h>
#include <pix3.h>
#include <array>

#include "defined.h"
#include "dxres.h"
#include "dxutil.h"

const char g_PositionSemanticName[] = "POSITION";
const char g_NormalSemanticName[] = "NORMAL";
const char g_TangentSemanticName[] = "TANGENT";
const char g_BinormalSemanticName[] = "BINORMAL";
const char g_UVSemanticName[] = "TEXCOORD";
const char g_BoneIndicesSemanticName[] = "BONEIDNCIES";
const char g_BoneWeightsSemanticName[] = "BONEWEIGHTS";

int BitSizeOfFormatElement(DXGI_FORMAT format);
int ByteSizeOfFormatElement(DXGI_FORMAT format);
bool EqualInputElementDesc(int descCount, D3D11_INPUT_ELEMENT_DESC* descArray0, D3D11_INPUT_ELEMENT_DESC* descArray1);

HRESULT LoadDX11Resoureces(DX11Resources* res, DX11RawResourceBuffer* rawBuffer, DX11ResourceDesc* desc, const Allocaters* allocs, ID3D11Device* device)
{
	HRESULT hr = S_OK;

	hr = LoadGeometryAndAnimationFromFBXChunk(res, rawBuffer, allocs, device, desc->fbxChunkCount, desc->fbxChunks, desc->fbxMeshConfigs);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to load geometries by FBX..");

	hr = LoadTexture2DAndSRVFromDirectories(
		res, rawBuffer, allocs, device,
		desc->textureDirCount, desc->texturedirs, desc->textureBufferSize, desc->allocatedtextureBuffer
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to load textures..");

	DX11CompileDescToShader* dtosBuffer = (DX11CompileDescToShader*)alloca(sizeof(DX11CompileDescToShader) * desc->shaderCompileCount);;

	hr = LoadShaderFromDirectories(
		res, rawBuffer, allocs, device, 
		desc->shaderCompileCount, desc->shaderCompileDescs, dtosBuffer
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to compile shaders..");

	hr = LinkInputLayout(
		res, rawBuffer, allocs,
		desc->shaderCompileCount, dtosBuffer, desc->inputLayoutCount, desc->inputLayoutDescs
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create dx11 input layout..");

	REALLOC_RANGE_ZEROMEM(
		prevConstantBufferCount, res->constantBufferCount, desc->constantBufferCount,
		uint, res->constantBufferIndices, allocs->realloc
	);	
	uint count = AppendConstantBuffers(rawBuffer, desc->constantBufferCount, desc->constantBufferSizes);
	for (uint i = 0; i < desc->constantBufferCount; i++)
		res->constantBufferIndices[i + prevConstantBufferCount] = count + i;

	count = AppendSamplerStates(rawBuffer, desc->samplerCount, desc->samplerDescs);
	// TODO:: sampler index out

	FAILED_ERROR_MESSAGE_RETURN(
		CreateDX11RawResourcesByDesc(res, rawBuffer, allocs, device, true), 
		L"fail to create dx11 samplers.."
	);

	return hr;
}

HRESULT ReleaseResources(DX11Resources* res, const Allocaters* allocs)
{
	SAFE_DEALLOC(res->geometryChunks, allocs->dealloc);

	if (res->vertexLayouts)
	{
		for (uint i = 0; i < res->vertexLayoutCount; i++)
			SAFE_DEALLOC(res->vertexLayouts[i].descs, allocs->dealloc);

		allocs->dealloc(res->vertexLayouts);
		res->vertexLayouts = nullptr;
	}

	if (res->texture2Ds)
	{
		for (uint i = 0; i < res->texture2DCount; i++)
			SAFE_RELEASE(res->texture2Ds[i].texture);

		allocs->dealloc(res->texture2Ds);
		res->texture2Ds = nullptr;
	}

	if (res->buffers)
	{
		for (uint i = 0; i < res->bufferCount; i++)
			SAFE_RELEASE(res->buffers[i]);

		allocs->dealloc(res->buffers);
		res->buffers = nullptr;
	}

	if (res->samplerStates)
	{
		for (uint i = 0; i < res->samplerCount; i++)
			SAFE_RELEASE(res->samplerStates[i]);

		allocs->dealloc(res->samplerStates);
		res->samplerStates = nullptr;
	}

	if (res->srvs)
	{
		for (uint i = 0; i < res->srvCount; i++)
			SAFE_RELEASE(res->srvs[i]);

		allocs->dealloc(res->srvs);
		res->srvs = nullptr;
	}

	if (res->uavs)
	{
		for (uint i = 0; i < res->uavCount; i++)
			SAFE_RELEASE(res->uavs[i]);

		allocs->dealloc(res->uavs);
		res->uavs = nullptr;
	}

	if (res->shaderFiles)
	{
		for (uint i = 0; i < res->shaderFileCount; i++)
		{
			DX11Resources::DX11ShaderFile* file = res->shaderFiles + i;
			for (uint j = 0; j < 6; j++)
				SAFE_DEALLOC(file->shaderIndices[j].indices, allocs->dealloc);
		}

		allocs->dealloc(res->shaderFiles);
	}

	for (uint i = 0; i < 6; i++)
		for (uint j = 0; j < res->shadersByKind[i].shaderCount; j++)
		{
			SAFE_RELEASE(res->shadersByKind[i].shaders[j].shaderBlob);
			SAFE_RELEASE(res->shadersByKind[i].shaders[j].vs);
		}

	return S_OK;
}


void SetDX11InputDescWithChunk(bool includeBone, int* descCounts, int* vertexSize/*[2]*/, D3D11_INPUT_ELEMENT_DESC* descBuffer, const FBXMeshChunk::FBXGeometryChunk* c)
{
	float realAlignment = 0, vertexDataSize = 0;
	vertexSize[0] = 0;
	vertexSize[1] = 0;
	descCounts[0] = 0;

	descBuffer[descCounts[0]].SemanticName = g_PositionSemanticName;
	descBuffer[descCounts[0]].SemanticIndex = 0;
	descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	descBuffer[descCounts[0]].InputSlot = 0;
	descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
	descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	descBuffer[descCounts[0]].InstanceDataStepRate = 0;

	realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
	descCounts[0]++;

	if (c->normals)
	{
		descBuffer[descCounts[0]].SemanticName = g_NormalSemanticName;
		descBuffer[descCounts[0]].SemanticIndex = 0;
		descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descBuffer[descCounts[0]].InputSlot = 0;
		descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
		descCounts[0]++;
	}
	if (c->tangents)
	{
		descBuffer[descCounts[0]].SemanticName = g_TangentSemanticName;
		descBuffer[descCounts[0]].SemanticIndex = 0;
		descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descBuffer[descCounts[0]].InputSlot = 0;
		descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
		descCounts[0]++;
	}
	if (c->binormals)
	{
		descBuffer[descCounts[0]].SemanticName = g_BinormalSemanticName;
		descBuffer[descCounts[0]].SemanticIndex = 0;
		descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descBuffer[descCounts[0]].InputSlot = 0;
		descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
		descCounts[0]++;
	}
	for (uint uvi = 0; uvi < c->uvSlotCount; uvi++)
	{
		if (c->uvSlots[uvi])
		{
			descBuffer[descCounts[0]].SemanticName = g_UVSemanticName;
			descBuffer[descCounts[0]].SemanticIndex = uvi;
			descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32_FLOAT;
			descBuffer[descCounts[0]].InputSlot = 0;
			descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
			descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			descBuffer[descCounts[0]].InstanceDataStepRate = 0;

			realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
			descCounts[0]++;
		}
	}
	if (includeBone && (c->boneIndices && c->boneWeights))
	{
		//descBuffer[descCounts[0]].SemanticName = g_BoneIndicesSemanticName;
		//descBuffer[descCounts[0]].SemanticIndex = 0;
		//descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32A32_UINT;
		//descBuffer[descCounts[0]].InputSlot = 0;
		//descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		//descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		//descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		vertexDataSize += ByteSizeOfFormatElement(DXGI_FORMAT_R32G32B32A32_UINT);
		//descCounts[0]++;

		//descBuffer[descCounts[0]].SemanticName = g_BoneWeightsSemanticName;
		//descBuffer[descCounts[0]].SemanticIndex = 0;
		//descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		//descBuffer[descCounts[0]].InputSlot = 0;
		//descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		//descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		//descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		vertexDataSize += ByteSizeOfFormatElement(DXGI_FORMAT_R32G32B32A32_FLOAT);
		//descCounts[0]++;
	}

	vertexSize[0] = realAlignment + vertexDataSize;
	vertexSize[1] = realAlignment;
}

int FindEqualDescIndex(uint descCount, D3D11_INPUT_ELEMENT_DESC* descBuffer, uint vertexLayoutBufferCount, DX11Resources::DX11LayoutChunk* vertexLayoutBuffer, DX11Resources* res)
{
	for (int vli = 0; vli < (int)res->vertexLayoutCount; vli++)
		if (
			res->vertexLayouts[vli].descCount == descCount &&
			EqualInputElementDesc(descCount, descBuffer, res->vertexLayouts[vli].descs)
			)
			return vli;

	
	for (int vli = 0; vli < (int)vertexLayoutBufferCount; vli++)
		if (
			vertexLayoutBuffer[vli].descCount == descCount &&
			EqualInputElementDesc(descCount, descBuffer, vertexLayoutBuffer[vli].descs)
			)
			return vli + res->vertexLayoutCount;

	return -1;
}

HRESULT LoadGeometryAndAnimationFromFBXChunk(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device,
	uint chunkCount, const FBXChunk* chunks, const FBXChunkConfig* configs
)
{
	int startLayoutCount = res->vertexLayoutCount,
		newGeometryCount = 0, newBufferCount = 0, newSRVUAVCount = 0;
	for (uint ci = 0; ci < chunkCount; ci++)
		for (uint mi = 0; mi < chunks[ci].meshCount; mi++)
		{
			if (configs[ci].meshConfigs[mi].isSkinned)
			{
				newBufferCount += 3;
				newSRVUAVCount++;
			}
			else
				newBufferCount += 2;

			newGeometryCount++;
		}

	REALLOC_RANGE_ZEROMEM(
		startGeometryCount, res->geometryCount, newGeometryCount,
		DX11Resources::DX11GeometryChunk, res->geometryChunks, allocs->realloc
	);

	int vertexLayoutBufferCount = 0;
	DX11Resources::DX11LayoutChunk* vertexLayoutBuffer =
		(DX11Resources::DX11LayoutChunk*)alloca(
			sizeof(DX11Resources::DX11LayoutChunk) * (res->geometryCount - startGeometryCount)
		);

	const int descBufferCapacity = 32;
	D3D11_INPUT_ELEMENT_DESC* descBuffer = (D3D11_INPUT_ELEMENT_DESC*)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity);
	memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity);

	for (
		uint ci = 0, geometryOffset = startGeometryCount, bufferOffset = 0, uavOffset = 0, srvOffset = 0; 
		ci < chunkCount; 
		ci++, memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity)
		)
	{
		const FBXChunk& c = chunks[ci];

#pragma region load meshes from FBX
		for (uint mi = 0; mi < c.meshCount; mi++, memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity))
		{
			FBXChunkConfig::FBXMeshConfig& mc = configs[ci].meshConfigs[mi];
			FBXMeshChunk& m = c.meshs[mi];
			DX11Resources::DX11GeometryChunk& g = res->geometryChunks[geometryOffset];

			// vertexlayout record start 

			int descCount, vertexSizes[2];			
			SetDX11InputDescWithChunk(mc.isSkinned, &descCount, vertexSizes, descBuffer, &m.geometry);
			int vertexSize = mc.isSkinned ? vertexSizes[1] : vertexSizes[0];

			int findVertexLayoutIndex = FindEqualDescIndex(descCount, descBuffer, vertexLayoutBufferCount, vertexLayoutBuffer, res);

			if (findVertexLayoutIndex < 0)
			{
				vertexLayoutBuffer[vertexLayoutBufferCount].vertexSize = vertexSize;
				vertexLayoutBuffer[vertexLayoutBufferCount].descCount = descCount;
				vertexLayoutBuffer[vertexLayoutBufferCount].descs =
					(D3D11_INPUT_ELEMENT_DESC*)allocs->alloc(
						sizeof(D3D11_INPUT_ELEMENT_DESC) * descCount
					);

				memcpy(
					vertexLayoutBuffer[vertexLayoutBufferCount].descs,
					descBuffer,
					sizeof(D3D11_INPUT_ELEMENT_DESC) * descCount
				);
				findVertexLayoutIndex = vertexLayoutBufferCount;
				vertexLayoutBufferCount++;
			}
			// vertexlayout record end / remain layout index for geomtry buffer

			// geometry create buffer start
			g.vertexLayoutIndex = findVertexLayoutIndex;
			g.indexCount = m.geometry.indexCount;
			g.vertexCount = m.geometry.vertexCount;
			g.isSkinned = configs[ci].meshConfigs[mi].isSkinned;

			auto vertexCopy = [=](void* vptr) -> void
			{
				byte* ptr = static_cast<byte*>(vptr);
				for (uint i = 0; i < g.vertexCount; i++)
				{
					memcpy(ptr, m.geometry.vertices + i, sizeof(m.geometry.vertices[i]));
					ptr += sizeof(m.geometry.vertices[i]);
					if (m.geometry.normals)
					{
						memcpy(ptr, m.geometry.normals + i, sizeof(m.geometry.normals[i]));
						ptr += sizeof(m.geometry.normals[i]);
					}
					if (m.geometry.tangents)
					{
						memcpy(ptr, m.geometry.tangents + i, sizeof(m.geometry.tangents[i]));
						ptr += sizeof(m.geometry.tangents[i]);
					}
					if (m.geometry.binormals)
					{
						memcpy(ptr, m.geometry.binormals + i, sizeof(m.geometry.binormals[i]));
						ptr += sizeof(m.geometry.binormals[i]);
					}
					for (uint j = 0; j < m.geometry.uvSlotCount; j++)
						if (m.geometry.uvSlots[j])
						{
							memcpy(ptr, m.geometry.uvSlots[j] + i, sizeof(m.geometry.uvSlots[j][i]));
							ptr += sizeof(m.geometry.uvSlots[j][i]);
						}
					if (m.geometry.boneIndices && m.geometry.boneWeights)
					{
						memcpy(ptr, m.geometry.boneIndices + i, sizeof(m.geometry.boneIndices[i]));
						ptr += sizeof(m.geometry.boneIndices[i]);
						memcpy(ptr, m.geometry.boneWeights + i, sizeof(m.geometry.boneWeights[i]));
						ptr += sizeof(m.geometry.boneWeights[i]);
					}
				}
			};

			if (!configs[ci].meshConfigs[mi].isSkinned)
			{
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = vertexSizes[0] * g.vertexCount;
					desc.buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;
					desc.copyToPtr = vertexCopy;

					g.vertexBufferIndex = AppendBuffer(rawBuffer, &desc);
				}
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = sizeof(m.geometry.indices[0]) * m.geometry.indexCount;
					desc.buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;
					desc.subres.pSysMem = m.geometry.indices;

					g.indexBufferIndex = AppendBuffer(rawBuffer, &desc);
				}
			}
			else
			{
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = vertexSizes[0] * g.vertexCount;
					desc.buffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					desc.buffer.CPUAccessFlags = 0;
					desc.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
					desc.buffer.StructureByteStride = vertexSizes[0];
					desc.copyToPtr = vertexCopy;

					g.vertexDataBufferIndex = AppendBuffer(rawBuffer, &desc);
				}

				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = sizeof(m.geometry.indices[0]) * m.geometry.indexCount;
					desc.buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;
					desc.subres.pSysMem = m.geometry.indices;
					g.indexBufferIndex = AppendBuffer(rawBuffer, &desc);
				}

				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = vertexSizes[1] * g.vertexCount;
					desc.buffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
					desc.buffer.CPUAccessFlags = 0;
					desc.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
					desc.buffer.StructureByteStride = vertexSizes[1];

					g.vertexStreamBufferIndex = AppendBuffer(rawBuffer, &desc);
				}
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = vertexSizes[1] * g.vertexCount;
					desc.buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;

					g.vertexBufferIndex = AppendBuffer(rawBuffer, &desc);
				}

				{
					DX11SRVDesc desc;
					memset(&desc, 0, sizeof(DX11SRVDesc));

					desc.bufferIndex = g.vertexDataBufferIndex;
					desc.view.Format = DXGI_FORMAT_UNKNOWN;
					desc.view.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
					desc.view.Buffer.FirstElement = 0;
					desc.view.Buffer.NumElements = g.vertexCount;

					g.vertexDataSRVIndex = AppendShaderResourceView(rawBuffer, &desc);
				}

				{
					DX11UAVDesc desc;
					memset(&desc, 0, sizeof(DX11UAVDesc));

					desc.bufferIndex = g.vertexStreamBufferIndex;
					desc.view.Format = DXGI_FORMAT_UNKNOWN;
					desc.view.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_BUFFER;
					desc.view.Buffer.FirstElement = 0;
					desc.view.Buffer.NumElements = g.vertexCount;
					desc.view.Buffer.Flags = 0;

					g.vertexStreamUAVIndex = AppendUnorderedAccessViews(rawBuffer, &desc);
				}
			}

			// geometry create end

			geometryOffset++;
		}

#pragma endregion
#pragma region load animations from fBX

		res->boneCount = c.hierarchyCount;

		REALLOC_RANGE_ZEROMEM(
			prevAnimCount, res->animCount, c.animationCount,
			DX11Resources::DX11Animation, res->anims, allocs->realloc
		);

		for (uint ai = prevAnimCount; ai < res->animCount; ai++)
		{
			DX11Resources::DX11Animation& anim = res->anims[ai];
			FBXChunk::FBXAnimation& fbxAnim = c.animations[ai - prevAnimCount];

			ALLOC_AND_STRCPY(anim.animName, fbxAnim.animationName, allocs->alloc);			
			anim.fpsCount = fbxAnim.fpsCount;
			anim.frameKeyCount = fbxAnim.frameKeyCount;

			DX11BufferDesc bd;
			memset(&bd, 0, sizeof(DX11BufferDesc));

			bd.buffer.Usage = D3D11_USAGE_DEFAULT;
			bd.buffer.ByteWidth = sizeof(Matrix4x4) * fbxAnim.frameKeyCount * c.hierarchyCount;
			bd.buffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bd.buffer.CPUAccessFlags = 0;
			bd.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bd.buffer.StructureByteStride = sizeof(Matrix4x4);
			bd.subres.pSysMem = fbxAnim.globalAffineTransforms;

			anim.animPoseTransformBufferIndex = AppendBuffer(rawBuffer, &bd);

			DX11SRVDesc srvd;
			memset(&srvd, 0, sizeof(DX11SRVDesc));

			srvd.bufferIndex = anim.animPoseTransformBufferIndex;
			srvd.view.Format = DXGI_FORMAT_UNKNOWN;
			srvd.view.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
			srvd.view.Buffer.FirstElement = 0;
			srvd.view.Buffer.NumElements = fbxAnim.frameKeyCount * c.hierarchyCount;

			anim.animPoseTransformSRVIndex = AppendShaderResourceView(rawBuffer, &srvd);
		}

		DX11BufferDesc bd;
		memset(&bd, 0, sizeof(DX11BufferDesc));

		bd.buffer.Usage = D3D11_USAGE_DEFAULT;
		bd.buffer.ByteWidth = sizeof(Matrix4x4) * c.hierarchyCount;
		bd.buffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.buffer.CPUAccessFlags = 0;
		bd.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.buffer.StructureByteStride = sizeof(Matrix4x4);
		bd.copyToPtr = [&](void* ptr) {
			Matrix4x4* matrixBuffer = static_cast<Matrix4x4*>(ptr);
			for (uint i = 0; i < c.hierarchyCount; i++)
				matrixBuffer[i] = c.hierarchyNodes[i].inverseGlobalTransformMatrix;
		};

		res->bindPoseTransformBufferIndex = AppendBuffer(rawBuffer, &bd);

		DX11SRVDesc srvd;
		memset(&srvd, 0, sizeof(DX11SRVDesc));

		srvd.bufferIndex = res->bindPoseTransformBufferIndex;
		srvd.view.Format = DXGI_FORMAT_UNKNOWN;
		srvd.view.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
		srvd.view.Buffer.FirstElement = 0;
		srvd.view.Buffer.NumElements = c.hierarchyCount;

		res->binePoseTransformSRVIndex = AppendShaderResourceView(rawBuffer, &srvd);

#pragma endregion

	}

	// allocate & copy vertex layout
	if (vertexLayoutBufferCount > 0)
	{
		res->vertexLayouts =
			(DX11Resources::DX11LayoutChunk*)allocs->realloc(
				res->vertexLayouts,
				sizeof(DX11Resources::DX11LayoutChunk) *
				(res->vertexLayoutCount + vertexLayoutBufferCount)
			);
		memcpy(
			res->vertexLayouts + res->vertexLayoutCount,
			vertexLayoutBuffer,
			sizeof(DX11Resources::DX11LayoutChunk) * vertexLayoutBufferCount
		);
		res->vertexLayoutCount += vertexLayoutBufferCount;
	}

	return S_OK;
}

inline size_t GetFileSize(FILE* fp)
{
	size_t size;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

HRESULT LoadTexture2DAndSRVFromDirectories(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device, 
	uint dirCount, const wchar_t** dirs, uint textureBufferSize, void* allocatedtextureBuffer
)
{
	ASSERT(dirs != nullptr);

	size_t currentTextureBufferSize = textureBufferSize ? textureBufferSize : 2024 * 2024 * 4 * 4;
	void* textureBuffer = allocatedtextureBuffer? allocatedtextureBuffer: allocs->alloc(currentTextureBufferSize);
	int textureBufferArrayCapacity = dirCount;

	res->texture2Ds = 
		(DX11Resources::DX11Texture2D*)allocs->realloc(
			res->texture2Ds, 
			sizeof(DX11Resources::DX11Texture2D) * (textureBufferArrayCapacity + res->texture2DCount)
		);
	memset(
		res->texture2Ds + res->texture2DCount, 
		0,
		sizeof(DX11Resources::DX11Texture2D) * (textureBufferArrayCapacity + res->texture2DCount)
	);

	FILE* fp; size_t size;
	for (uint i = 0; i < dirCount; i++)
	{
		if (dirs[i] == nullptr)
			continue;

		_wfopen_s(&fp, dirs[i], L"rb");
		size = GetFileSize(fp);
		if (currentTextureBufferSize < size)
		{
			currentTextureBufferSize = size;
			textureBuffer = allocs->realloc(textureBuffer, currentTextureBufferSize);
		}
		size_t readSize = fread(textureBuffer, 1, size, fp);
		fclose(fp);
		fp = nullptr;

		if (readSize != size)
			continue;

		DirectX::ScratchImage img;
		FAILED_WARN_MESSAGE_CONTINUE_ARGS(
			DirectX::LoadFromWICMemory(textureBuffer, size, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, nullptr, img), 
			L"fail to load image from file:\"%s\"..",
			dirs[i]
			);

		FAILED_WARN_MESSAGE_CONTINUE(
			DirectX::CreateTextureEx(
				device,
				img.GetImages(), img.GetImageCount(), img.GetMetadata(),
				D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false,
				reinterpret_cast<ID3D11Resource**>(&res->texture2Ds[i + res->texture2DCount].texture)
			),
			L"fail to create d3d11 texture2d object.."
		);

		D3D11_TEXTURE2D_DESC td;
		res->texture2Ds[i + res->texture2DCount].texture->GetDesc(&td);

		DX11SRVDesc desc;
		memset(&desc, 0, sizeof(desc));
		desc.texture2DIndex = i + res->texture2DCount;
		desc.view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.view.Format = td.Format;
		desc.view.Texture2D.MipLevels = 1;
		desc.view.Texture2D.MostDetailedMip = 0;

		res->texture2Ds[i + res->texture2DCount].srvIndex = AppendShaderResourceView(
			rawBuffer, &desc
		);
	}

	res->texture2DCount += dirCount;
	SAFE_DEALLOC(textureBuffer, allocs->dealloc);

	return S_OK;
}

HRESULT LoadShaderFromDirectories(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device, 
	uint compileCount, const DX11ShaderCompileDesc* descs, DX11CompileDescToShader* descToFileShader
)
{
	std::vector<DX11Resources::DX11ShaderFile> files = std::vector<DX11Resources::DX11ShaderFile>();
	std::vector<std::array<std::vector<uint>, 6>> shaderIndicesByFile = std::vector<std::array<std::vector<uint>, 6>>();

	for (uint i = 0; i < compileCount; i++)
	{
		const DX11ShaderCompileDesc& dsc = descs[i];
		int fileIndex = -1;
		for (int j = 0; j < files.size(); j++)
			if (wcscmp(files[j].fileName, dsc.fileName) == 0)
			{
				fileIndex = j;
				break;
			}

		if (fileIndex < 0)
		{
			DX11Resources::DX11ShaderFile file;
			memset(&file, 0, sizeof(DX11Resources::DX11ShaderFile));

			size_t len = wcslen(dsc.fileName);
			wchar_t* tempFileName = (wchar_t*)allocs->alloc(sizeof(wchar_t) * (len + 1));
			wcscpy_s(tempFileName, (len + 1), dsc.fileName);
			file.fileName = tempFileName;

			fileIndex = (int)files.size();
			files.push_back(file);
			std::array<std::vector<uint>, 6> arr;

			shaderIndicesByFile.push_back(arr);
		}

		DX11Resources::DX11ShaderFile& file = files[fileIndex];
		ShaderKind s;
		uint index = AppendShader(rawBuffer, &dsc, &s);
		if (index == UINT_MAX)
			continue;

		descToFileShader[i].shaderKindIndex = (uint)s;
		descToFileShader[i].shaderIndex = index;
		shaderIndicesByFile[fileIndex][(int)s].push_back(index);
	}

	REALLOC_RANGE_MEMCPY(
		prevShaderFileCount, res->shaderFileCount, files.size(),
		DX11Resources::DX11ShaderFile, res->shaderFiles, files.data(), allocs->realloc
	);

	for (uint i = 0; i < res->shaderFileCount; i++)
	{
		DX11Resources::DX11ShaderFile& file = res->shaderFiles[i];

		for (int j = 0; j < 6; j++)
		{
			std::vector<uint>& v = shaderIndicesByFile[i][j];
			file.shaderIndices[j].count = (uint)v.size();
			file.shaderIndices[j].indices = (uint*)allocs->realloc(file.shaderIndices[j].indices, sizeof(uint) * v.size());
			memcpy(file.shaderIndices[j].indices, shaderIndicesByFile[i][(int)j].data(), sizeof(uint) * v.size());
		}			
	}

	return S_OK;
}

HRESULT LinkInputLayout(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs,
	uint descCount, const DX11CompileDescToShader* dtoss,
	uint inputLayoutCount, const DX11InputLayoutDesc* inputLayoutDesc
)
{
	REALLOC_RANGE_ZEROMEM(
		prevInputLayoutCount, res->inputLayoutCount, inputLayoutCount,
		DX11Resources::DX11InputLayout, res->inputLayouts, allocs->realloc
	);

	for (uint i = prevInputLayoutCount; i < res->inputLayoutCount; i++)
	{
		const DX11InputLayoutDesc& d = inputLayoutDesc[i];
		const DX11CompileDescToShader& dtof = dtoss[d.shaderCompileDescIndex];
		DX11ILDesc desc;
		desc.vertexLayoutChunkIndex = d.layoutChunkIndex;
		desc.vertexShaderIndex = dtof.shaderIndex;

		res->inputLayouts[i].inputLayoutIndex = AppendInputLayout(rawBuffer, desc);
		res->inputLayouts[i].vertexShaderIndex = dtof.shaderIndex;
		res->inputLayouts[i].layoutChunkIndex = d.layoutChunkIndex;
	}

	return S_OK;
}

uint AppendInputLayouts(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11ILDesc* descs)
{
	rawResBuffer->inputLayoutDescs.insert(rawResBuffer->inputLayoutDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->inputLayoutDescs.size() - additioanlCount;
}
uint AppendConstantBuffers(DX11RawResourceBuffer* rawResBuffer, uint constantBufferCount, const uint* bufferSizes)
{
	for (uint i = 0; i < constantBufferCount; i++)
	{
		DX11BufferDesc desc;
		memset(&desc, 0, sizeof(DX11BufferDesc));
		desc.buffer.Usage = D3D11_USAGE_DEFAULT;
		desc.buffer.ByteWidth = bufferSizes[i];
		desc.buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.buffer.CPUAccessFlags = 0;
		rawResBuffer->bufferDescs.push_back(desc);
	}

	return (uint)rawResBuffer->bufferDescs.size() - constantBufferCount;
}
void AppendShaders(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11ShaderCompileDesc* descs, OUT ShaderKind* kinds, OUT int* indices)
{
	for (uint i = 0; i < additioanlCount; i++)
	{
		if (indices) indices[i] = -1;
		int index = ShaderTargetToIndex(descs[i].target[0]);
		FALSE_ERROR_MESSAGE_CONTINUE(index >= 0, L"fail to identify shader by target..");
		if (kinds) kinds[i] = (ShaderKind)index;
		if (indices) indices[i] = (int)rawResBuffer->shaderCompileDesces[index].size();
		rawResBuffer->shaderCompileDesces[index].push_back(descs[i]);
	}
}
uint AppendSamplerStates(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const D3D11_SAMPLER_DESC* descs)
{
	rawResBuffer->samplerDescs.insert(rawResBuffer->samplerDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->samplerDescs.size() - additioanlCount;
}
uint AppendShaderResourceViews(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11SRVDesc* descs)
{
	rawResBuffer->srvDescs.insert(rawResBuffer->srvDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->srvDescs.size() - additioanlCount;
}
uint AppendUnorderedAccessViews(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11UAVDesc* descs)
{
	rawResBuffer->uavDescs.insert(rawResBuffer->uavDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->uavDescs.size() - additioanlCount;
}
uint AppendBuffers(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11BufferDesc* descs)
{
	rawResBuffer->bufferDescs.insert(rawResBuffer->bufferDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->bufferDescs.size() - additioanlCount;
}
uint AppendTex2Ds(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11Texture2DDesc* descs)
{
	rawResBuffer->tex2DDescs.insert(rawResBuffer->tex2DDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->tex2DDescs.size() - additioanlCount;
}

uint AppendInputLayout(DX11RawResourceBuffer* rawResBuffer, const DX11ILDesc& desc)
{
	rawResBuffer->inputLayoutDescs.push_back(desc);
	return (uint)rawResBuffer->inputLayoutDescs.size() - 1;
}
uint AppendConstantBuffer(DX11RawResourceBuffer* rawResBuffer, const uint bufferSize)
{
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bufferSize;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	DX11BufferDesc desc;
	memset(&desc, 0, sizeof(DX11BufferDesc));
	desc.buffer = bufferDesc;
	rawResBuffer->bufferDescs.push_back(desc);
	return (uint)rawResBuffer->bufferDescs.size() - 1;
}
uint AppendSamplerState(DX11RawResourceBuffer* rawResBuffer, const D3D11_SAMPLER_DESC* desc)
{
	rawResBuffer->samplerDescs.push_back(*desc);
	return (uint)rawResBuffer->samplerDescs.size() - 1;
}
uint AppendShader(DX11RawResourceBuffer* rawResBuffer, const DX11ShaderCompileDesc* desc, OUT ShaderKind* s)
{
	int index = ShaderTargetToIndex(desc->target[0]);
	FALSE_ERROR_MESSAGE_RETURN_CODE(index >= 0, L"fail to identify shader by target..", UINT_MAX);
	if (s) *s = (ShaderKind)index;
	rawResBuffer->shaderCompileDesces[index].push_back(*desc);
	return (uint)rawResBuffer->shaderCompileDesces[index].size() - 1;
}
uint AppendShaderResourceView(DX11RawResourceBuffer* rawResBuffer, const DX11SRVDesc* desc)
{
	rawResBuffer->srvDescs.push_back(*desc);
	return (uint)rawResBuffer->srvDescs.size() - 1;
}
uint AppendUnorderedAccessViews(DX11RawResourceBuffer* rawResBuffer, const DX11UAVDesc* desc)
{
	rawResBuffer->uavDescs.push_back(*desc);
	return (uint)rawResBuffer->uavDescs.size() - 1;
}
uint AppendBuffer(DX11RawResourceBuffer* rawResBuffer, const DX11BufferDesc* desc)
{
	rawResBuffer->bufferDescs.push_back(*desc);
	return (uint)rawResBuffer->bufferDescs.size() - 1;
}
uint AppendTex2D(DX11RawResourceBuffer* rawResBuffer, const DX11Texture2DDesc* desc)
{
	rawResBuffer->tex2DDescs.push_back(*desc);
	return (uint)rawResBuffer->tex2DDescs.size() - 1;
}

HRESULT CreateDX11RawResourcesByDesc(DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device, bool isDebug)
{
	uint prevShaderCount[6] = { 0, 0, 0,0 ,0 ,0 };
	for (uint i = 0; i < 6; i++)
	{
		auto& shaderChunks = res->shadersByKind[i];
		auto& shaderDescs = rawBuffer->shaderCompileDesces[i];

		REALLOC_EXISTVAL_RANGE_ZEROMEM(
			prevShaderCount[i], shaderChunks.shaderCount, rawBuffer->shaderCompileDesces[i].size(),
			DX11CompiledShader, shaderChunks.shaders, allocs->realloc
		);

		for (uint j = 0; j < shaderChunks.shaderCount; j++)
		{
			auto& desc = shaderDescs[j];
			auto& shader = shaderChunks.shaders[j];

			ALLOC_AND_STRCPY(shader.target, desc.target, allocs->alloc);
			ALLOC_AND_STRCPY(shader.entrypoint, desc.entrypoint, allocs->alloc);

			ID3DBlob* blob;
			FAILED_ERROR_MESSAGE_CONTINUE_ARGS(
				CompileShaderFromFile(desc.fileName, desc.entrypoint, desc.target, isDebug, &blob),
				L"fail to compile shader(%s,%s,%s)..",
				desc.fileName, desc.entrypoint, desc.target
			);
			shader.shaderBlob = blob;
			switch ((ShaderKind)i)
			{
			case ShaderKind::Vertex:
				FAILED_ERROR_MESSAGE_ARGS(
					device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shaderChunks.shaders[j].vs),
					L"fail to compile vertex shader(%s,%s,%s)..",
					desc.fileName, desc.entrypoint, desc.target
				);
				break;
			case ShaderKind::Pixel:
				FAILED_ERROR_MESSAGE_ARGS(
					device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shaderChunks.shaders[j].ps),
					L"fail to compile pixel shader(%s,%s,%s)..",
					desc.fileName, desc.entrypoint, desc.target
				);
				break;
			case ShaderKind::Compute:
				FAILED_ERROR_MESSAGE_ARGS(
					device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shaderChunks.shaders[j].cs),
					L"fail to compile compute shader(%s,%s,%s)..",
					desc.fileName, desc.entrypoint, desc.target
				);
				break;
			case ShaderKind::Geometry:
				FAILED_ERROR_MESSAGE_ARGS(
					device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shaderChunks.shaders[j].gs),
					L"fail to compile geometry shader(%s,%s,%s)..",
					desc.fileName, desc.entrypoint, desc.target
				);
				break;
			case ShaderKind::Hull:
				FAILED_ERROR_MESSAGE_ARGS(
					device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shaderChunks.shaders[j].hs),
					L"fail to compile hull shader(%s,%s,%s)..",
					desc.fileName, desc.entrypoint, desc.target
				);
				break;
			case ShaderKind::Doamin:
				FAILED_ERROR_MESSAGE_ARGS(
					device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shaderChunks.shaders[j].ds),
					L"fail to compile domain shader(%s,%s,%s)..",
					desc.fileName, desc.entrypoint, desc.target
				);
				break;
			}

		}
	}

	REALLOC_RANGE_ZEROMEM(
		prevSamplerCount, res->samplerCount, rawBuffer->samplerDescs.size(),
		ID3D11SamplerState*, res->samplerStates, allocs->realloc
	);
	for (uint i = prevSamplerCount; i < res->samplerCount; i++)
	{
		FAILED_ERROR_MESSAGE_ARGS(
			device->CreateSamplerState(&rawBuffer->samplerDescs[i], &res->samplerStates[i]),
			L"failed to create sampler(idx:%d)..",
			i
		);
	}

	REALLOC_RANGE_ZEROMEM(
		prevBufferCount, res->bufferCount, rawBuffer->bufferDescs.size(),
		ID3D11Buffer*, res->buffers, allocs->realloc
	);

	size_t prevSize = 0;
	void* ptr = nullptr;
	for (uint i = prevBufferCount; i < res->bufferCount; i++)
	{
		DX11BufferDesc& d = rawBuffer->bufferDescs[i];

		if (d.subres.pSysMem == nullptr)
		{
			if ((bool)d.copyToPtr)
			{
				if (prevSize < d.buffer.ByteWidth)
				{
					ptr = allocs->realloc(ptr, d.buffer.ByteWidth);
					prevSize = d.buffer.ByteWidth;
				}
				d.copyToPtr(ptr);
				d.subres.pSysMem = ptr;
			}
			else if (d.fileName)
			{
				FILE* fp;
				_wfopen_s(&fp, d.fileName, L"rb");
				size_t size = GetFileSize(fp);
				if (prevSize < size)
				{
					ptr = allocs->realloc(ptr, size);
					prevSize = size;
				}
				fread(ptr, 1, size, fp);
				fclose(fp);
				d.subres.pSysMem = ptr;
			}
			else if (d.bufferPtr)
				d.subres.pSysMem = d.bufferPtr;
		}

		FAILED_ERROR_MESSAGE_ARGS(
			device->CreateBuffer(&d.buffer, d.subres.pSysMem ? &d.subres : nullptr, &res->buffers[i]),
			L"fail to create buffer(idx:%d)..",
			i
		);
	}		
	SAFE_DEALLOC(ptr, allocs->dealloc);

	REALLOC_RANGE_ZEROMEM(
		prevSRVCount, res->srvCount, rawBuffer->srvDescs.size(),
		ID3D11ShaderResourceView*, res->srvs, allocs->realloc
	);
	for (uint i = prevSRVCount; i < res->srvCount; i++)
	{
		ID3D11Resource* dx11res = nullptr;

		switch (rawBuffer->srvDescs[i].view.ViewDimension)
		{
		case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER:
			dx11res = res->buffers[rawBuffer->srvDescs[i].bufferIndex];
			break;
		case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D:
			dx11res = res->texture2Ds[rawBuffer->srvDescs[i].texture2DIndex].texture;
			break;
		default:
			ERROR_MESSAGE_CONTINUE_ARGS(
				L"failed by corrupted view type:%d\n",
				rawBuffer->srvDescs[i].view.ViewDimension
			);
		}
		
		FAILED_ERROR_MESSAGE_ARGS(
			device->CreateShaderResourceView(dx11res, &rawBuffer->srvDescs[i].view, &res->srvs[i]),
			L"failed to create SRV(idx:%d)..",
			i
		);
	}
		
	REALLOC_RANGE_ZEROMEM(
		prevUAVCount, res->uavCount, rawBuffer->uavDescs.size(),
		ID3D11UnorderedAccessView*, res->uavs, allocs->realloc
	);
	for (uint i = prevUAVCount; i < res->uavCount; i++)
	{
		FAILED_ERROR_MESSAGE_ARGS(
			device->CreateUnorderedAccessView(res->buffers[rawBuffer->uavDescs[i].bufferIndex], &rawBuffer->uavDescs[i].view, &res->uavs[i]),
			L"failed to create UAV(idx:%d)..",
			i
		);
		
	}

	REALLOC_RANGE_ZEROMEM(
		prevILCount, res->inputLayoutItemCount, rawBuffer->inputLayoutDescs.size(),
		ID3D11InputLayout*, res->inputLayoutItems, allocs->realloc
	);
	for (uint i = prevILCount; i < res->inputLayoutItemCount; i++)
	{
		FAILED_ERROR_MESSAGE_ARGS(
			device->CreateInputLayout(
				res->vertexLayouts[rawBuffer->inputLayoutDescs[i].vertexLayoutChunkIndex].descs,
				res->vertexLayouts[rawBuffer->inputLayoutDescs[i].vertexLayoutChunkIndex].descCount,
				res->shaders.vss[rawBuffer->inputLayoutDescs[i].vertexShaderIndex].shaderBlob->GetBufferPointer(),
				res->shaders.vss[rawBuffer->inputLayoutDescs[i].vertexShaderIndex].shaderBlob->GetBufferSize(),
				res->inputLayoutItems + i
			),
			L"failed to create input layout(idx:%d)..",
			i
		)
	}

	return S_OK;
}

HRESULT UploadConstantBuffer(
	DX11Resources* res, ID3D11DeviceContext* deviceContext, uint constantBufferIndex, void* uploadData
)
{
	HRESULT hr = S_OK;
	deviceContext->UpdateSubresource(res->buffers[res->constantBufferIndices[constantBufferIndex]], 0, nullptr, uploadData, 0, 0);
	return hr;
}
HRESULT DependancyContextStatePrepare(DX11ContextState* state, const Allocaters* allocs, uint dependCount, const DX11PipelineDependancy* depends)
{
	uint maxCount = state->bufferCount;
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11PipelineDependancy& depend = depends[i];
		if (depend.pipelineKind == PIPELINE_KIND::DRAW)
		{
			for (int k = 0; k < 5; k++)
			{
				const DX11ShaderResourceDependancy& shaderDep = depend.draw.dependants[k];

				for (uint j = 0; j < shaderDep.constantBufferCount; j++)
				{
					auto ref = shaderDep.constantBuffers[j];
					maxCount = max(ref.indexCount, maxCount);
				}
				for (uint j = 0; j < shaderDep.samplerCount; j++)
				{
					auto ref = shaderDep.samplers[j];
					maxCount = max(ref.indexCount, maxCount);
				}
				for (uint j = 0; j < shaderDep.srvCount; j++)
				{
					auto ref = shaderDep.srvs[j];
					maxCount = max(ref.indexCount, maxCount);
				}
				for (uint j = 0; j < shaderDep.uavCount; j++)
				{
					auto ref = shaderDep.uavs[j];
					maxCount = max(ref.indexCount, maxCount);
				}
			}
		}
		else if (depend.pipelineKind == PIPELINE_KIND::COMPUTE)
		{
			const DX11ShaderResourceDependancy& shaderDep = depend.compute.resources;

			for (uint j = 0; j < shaderDep.constantBufferCount; j++)
			{
				auto ref = shaderDep.constantBuffers[j];
				maxCount = max(ref.indexCount, maxCount);
			}
			for (uint j = 0; j < shaderDep.samplerCount; j++)
			{
				auto ref = shaderDep.samplers[j];
				maxCount = max(ref.indexCount, maxCount);
			}
			for (uint j = 0; j < shaderDep.srvCount; j++)
			{
				auto ref = shaderDep.srvs[j];
				maxCount = max(ref.indexCount, maxCount);
			}
			for (uint j = 0; j < shaderDep.uavCount; j++)
			{
				auto ref = shaderDep.uavs[j];
				maxCount = max(ref.indexCount, maxCount);
			}
		}
	}

	if (state->bufferCount < maxCount)
	{
		state->bufferPtrBuffer = (void**)allocs->realloc(state->bufferPtrBuffer, sizeof(void*) * maxCount);
		state->numberBuffer = (uint*)allocs->realloc(state->numberBuffer, sizeof(uint) * 2);
	}
		
	return S_OK;
}
HRESULT Copy(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11CopyDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		switch (depends[i].kind)
		{
		case CopyKind::COPY_RESOURCE:
			deviceContext->CopyResource(res->buffers[depends[i].dstBufferIndex], res->buffers[depends[i].srcBufferIndex]);
			break;
		}
	}

	return S_OK;
}

HRESULT ExecuteExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		switch (depends[i].pipelineKind)
		{
		case PIPELINE_KIND::DRAW:
			DrawExplicitly(deviceContext, state, res, 1, &depends[i].draw);
			break;
		case PIPELINE_KIND::COMPUTE:
			ComputeExplicitly(deviceContext, state, res, 1, &depends[i].compute);
			break;
		case PIPELINE_KIND::COPY:
			Copy(deviceContext, state, res, 1, &depends[i].copy);
			break;
		}
	}

	return S_OK;
}
HRESULT ExecuteImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		switch (depends[i].pipelineKind)
		{
		case PIPELINE_KIND::DRAW:
			DrawImplicitly(deviceContext, state, res, 1, &depends[i].draw);
			break;
		case PIPELINE_KIND::COMPUTE:
			ComputeImplicitly(deviceContext, state, res, 1, &depends[i].compute);
			break;
		case PIPELINE_KIND::COPY:
			Copy(deviceContext, state, res, 1, &depends[i].copy);
			break;
		}
	}

	return S_OK;
}
HRESULT ComputeExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11ComputePipelineDependancy& cpd = depends[i];
		const DX11ShaderResourceDependancy& srd = cpd.resources;

		if (srd.shaderFileIndex < 0) continue;

		deviceContext->CSSetShader(res->shaders.css[res->shaderFiles[srd.shaderFileIndex].csIndices[srd.shaderIndex]], nullptr, 0);

		for (uint j = 0; j < srd.constantBufferCount; j++)
		{
			auto ref = srd.constantBuffers[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = res->buffers[res->constantBufferIndices[ref.indices[k]]];
			deviceContext->CSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
		}
		for (uint j = 0; j < srd.samplerCount; j++)
		{
			auto ref = srd.samplers[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
			deviceContext->CSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
		}
		for (uint j = 0; j < srd.srvCount; j++)
		{
			auto ref = srd.srvs[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
			deviceContext->CSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
		}
		for (uint j = 0; j < srd.uavCount; j++)
		{
			auto ref = srd.uavs[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = res->uavs[ref.indices[k]];
			// TODO:: set uav argument for append, consume buffer
			state->numberBuffer[0] = state->numberBuffer[1] = 0;
			deviceContext->CSSetUnorderedAccessViews(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11UnorderedAccessView *const *>(state->bufferPtrBuffer), state->numberBuffer);
		}

		switch (cpd.dispatchType)
		{
		case DX11ComputePipelineDependancy::DispatchType::DISPATCH:
			deviceContext->Dispatch(
				cpd.argsAsDispatch.dispatch.threadGroupCountX, 
				cpd.argsAsDispatch.dispatch.threadGroupCountY,
				cpd.argsAsDispatch.dispatch.threadGroupCountZ
				);
			break;
		case DX11ComputePipelineDependancy::DispatchType::DISPATCH_INDIRECT:
			deviceContext->DispatchIndirect(
				res->buffers[cpd.argsAsDispatch.dispatchIndirect.bufferIndex],
				cpd.argsAsDispatch.dispatchIndirect.bufferIndex
			);
			break;
		}
	}

	return S_OK;
}
HRESULT ComputeImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends)
{
	return E_NOTIMPL;
}

HRESULT DrawExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11DrawPipelineDependancy& depend = depends[i];
		const auto& depIn = depend.input;
		const auto& resGeo = res->geometryChunks[depIn.geometryIndex];

		deviceContext->IASetInputLayout(res->inputLayoutItems[res->inputLayouts[depIn.inputLayoutIndex].inputLayoutIndex]);
		deviceContext->IASetVertexBuffers(0, 1, &res->buffers[resGeo.vertexBufferIndex], &res->vertexLayouts[resGeo.vertexLayoutIndex].vertexSize, &depIn.vertexBufferOffset);
		deviceContext->IASetIndexBuffer(res->buffers[resGeo.indexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(depIn.topology);

		if (depend.vs.shaderFileIndex >= 0)
		{
			deviceContext->VSSetShader(res->shaders.vss[res->shaderFiles[depend.vs.shaderFileIndex].vsIndices[depend.vs.shaderIndex]], nullptr, 0);

			for (uint j = 0; j < depend.vs.constantBufferCount; j++)
			{
				auto ref = depend.vs.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->buffers[res->constantBufferIndices[ref.indices[k]]];
				deviceContext->VSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.vs.samplerCount; j++)
			{
				auto ref = depend.vs.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->VSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.vs.srvCount; j++)
			{
				auto ref = depend.vs.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->VSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.ps.shaderFileIndex >= 0)
		{
			deviceContext->PSSetShader(res->shaders.pss[res->shaderFiles[depend.ps.shaderFileIndex].psIndices[depend.ps.shaderIndex]], nullptr, 0);

			for (uint j = 0; j < depend.ps.constantBufferCount; j++)
			{
				auto ref = depend.ps.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->buffers[res->constantBufferIndices[ref.indices[k]]];
				deviceContext->PSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ps.samplerCount; j++)
			{
				auto ref = depend.ps.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->PSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ps.srvCount; j++)
			{
				auto ref = depend.ps.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->PSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}


		if (depend.gs.shaderFileIndex >= 0)
		{
			deviceContext->GSSetShader(res->shaders.gss[res->shaderFiles[depend.gs.shaderFileIndex].gsIndices[depend.gs.shaderIndex]], nullptr, 0);

			for (uint j = 0; j < depend.gs.constantBufferCount; j++)
			{
				auto ref = depend.gs.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->buffers[res->constantBufferIndices[ref.indices[k]]];
				deviceContext->GSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.gs.samplerCount; j++)
			{
				auto ref = depend.gs.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->GSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.gs.srvCount; j++)
			{
				auto ref = depend.gs.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->GSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.hs.shaderFileIndex >= 0)
		{
			deviceContext->HSSetShader(res->shaders.hss[res->shaderFiles[depend.hs.shaderFileIndex].hsIndices[depend.hs.shaderIndex]], nullptr, 0);

			for (uint j = 0; j < depend.hs.constantBufferCount; j++)
			{
				auto ref = depend.hs.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->buffers[res->constantBufferIndices[ref.indices[k]]];
				deviceContext->HSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.hs.samplerCount; j++)
			{
				auto ref = depend.hs.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->HSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.hs.srvCount; j++)
			{
				auto ref = depend.hs.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->HSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.ds.shaderFileIndex >= 0)
		{
			deviceContext->DSSetShader(res->shaders.dss[res->shaderFiles[depend.ds.shaderFileIndex].dsIndices[depend.ds.shaderIndex]], nullptr, 0);

			for (uint j = 0; j < depend.ds.constantBufferCount; j++)
			{
				auto ref = depend.ds.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->buffers[res->constantBufferIndices[ref.indices[k]]];
				deviceContext->DSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ds.samplerCount; j++)
			{
				auto ref = depend.ds.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->DSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ds.srvCount; j++)
			{
				auto ref = depend.ds.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->DSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		switch(depend.drawType)
		{
		case DX11DrawPipelineDependancy::DrawType::DRAW:
			deviceContext->Draw(
				depend.argsAsDraw.drawArgs.vertexCount, depend.argsAsDraw.drawArgs.startVertexLocation
			);
			break;
		case DX11DrawPipelineDependancy::DrawType::DRAW_INDEXED:
			deviceContext->DrawIndexed(
				depend.argsAsDraw.drawIndexedArgs.indexCount, 
				depend.argsAsDraw.drawIndexedArgs.startIndexLocation, 
				depend.argsAsDraw.drawIndexedArgs.baseVertexLocation
			);
			break;
		}
	}
	return S_OK;
}
HRESULT DrawImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends)
{
	return E_NOTIMPL;
}
HRESULT ReleaseDependancy(DX11PipelineDependancy* dependancy, const Allocaters* allocs)
{
	return E_NOTIMPL;
}
HRESULT ReleaseDrawDependancy(DX11DrawPipelineDependancy* dependancy, const Allocaters* allocs)
{
	return E_NOTIMPL;
}
HRESULT ReleaseComputeDependancy(DX11ComputePipelineDependancy* dependancy, const Allocaters* allocs)
{
	return E_NOTIMPL;
}
HRESULT ReleaseContext(DX11ContextState* context, const Allocaters* allocs)
{
	return E_NOTIMPL;
}

bool EqualInputElementDesc(int descCount, D3D11_INPUT_ELEMENT_DESC* descArray0, D3D11_INPUT_ELEMENT_DESC* descArray1)
{
	for (int i = 0; i < descCount; i++)
	{
		D3D11_INPUT_ELEMENT_DESC& d0 = descArray0[i], &d1 = descArray1[i];
		if (strcmp(d0.SemanticName, d1.SemanticName))
			return false;
		if (d0.SemanticIndex != d1.SemanticIndex)
			return false;
		if (d0.Format != d1.Format)
			return false;
		if (d0.InputSlot != d1.InputSlot)
			return false;
		if (d0.AlignedByteOffset != d1.AlignedByteOffset)
			return false;
		if (d0.InputSlotClass != d1.InputSlotClass)
			return false;
		if (d0.InstanceDataStepRate != d1.InstanceDataStepRate)
			return false;
	}

	return true;
}

int ByteSizeOfFormatElement(DXGI_FORMAT format)
{
	int size = BitSizeOfFormatElement(format);
	if (size < 0) return -1;
	else return size / 8;
}

//https://www.gamedev.net/forums/topic/523623-sizeof---dxgi_format/
int BitSizeOfFormatElement(DXGI_FORMAT format) {
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 64;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return 32;
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;
		// Compressed format; http://msdn2.microsoft.com/en-us/library/bb694531(VS.85).aspx        
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		return 128;
		// Compressed format; http://msdn2.microsoft.com/en-us/library/bb694531(VS.85).aspx        
	case DXGI_FORMAT_R1_UNORM:
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 64;
		// Compressed format; http://msdn2.microsoft.com/en-us/library/bb694531(VS.85).aspx        
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		return 32;
		// These are compressed, but bit-size information is unclear.        
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		return 32;
	case DXGI_FORMAT_UNKNOWN:
	default:
		return -1;
	}
}
