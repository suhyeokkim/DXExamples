#include <DirectXTex.h>
#include <vector>
#include <d3d11_4.h>
#include <pix3.h>

#include "dxres.h"
#include "dxutil.h"

HRESULT ReleaseResources(DX11Resources* res, const Allocaters* allocs)
{
	if (res->vertexLayouts)
	{
		for (uint i = 0; i < res->vertexLayoutCount; i++)
			SAFE_DEALLOC(res->vertexLayouts[i].descs, allocs->dealloc);

		allocs->dealloc(res->vertexLayouts);
		res->vertexLayouts = nullptr;
	}

	if (res->geometryChunks)
	{
		for (uint i = 0; i < res->geometryCount; i++)
		{
			SAFE_RELEASE(res->geometryChunks[i].vertexBuffer);
			SAFE_RELEASE(res->geometryChunks[i].indexBuffer);
		}

		allocs->dealloc(res->geometryChunks);
		res->geometryChunks = nullptr;
	}

	if (res->texture2Ds)
	{
		for (uint i = 0; i < res->texture2DCount; i++)
			SAFE_RELEASE(res->texture2Ds[i].texture);

		allocs->dealloc(res->texture2Ds);
		res->texture2Ds = nullptr;
	}

	if (res->constantBuffers)
	{
		for (uint i = 0; i < res->constantBufferCount; i++)
			SAFE_RELEASE(res->constantBuffers[i]);

		allocs->dealloc(res->constantBuffers);
		res->constantBuffers = nullptr;
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
		for (int i = 0; i < res->shaderFileCount; i++)
		{
			DX11Resources::DX11ShaderFile* file = res->shaderFiles + i;

			if (file->vsBlobs)
			{
				for (uint j = 0; j < file->vsBlobCount; j++)
					SAFE_RELEASE(file->vsBlobs[j].vs);
				allocs->dealloc(file->vsBlobs);
			}
			if (file->psBlobs)
			{
				for (uint j = 0; j < file->psBlobCount; j++)
					SAFE_RELEASE(file->psBlobs[j].ps);
				allocs->dealloc(file->psBlobs);
			}
			if (file->csBlobs)
			{
				for (uint j = 0; j < file->csBlobCount; j++)
					SAFE_RELEASE(file->csBlobs[j].cs);
				allocs->dealloc(file->csBlobs);
			}
			if (file->gsBlobs)
			{
				for (uint j = 0; j < file->gsBlobCount; j++)
					SAFE_RELEASE(file->gsBlobs[j].gs);
				allocs->dealloc(file->gsBlobs);
			}
			if (file->hsBlobs)
			{
				for (uint j = 0; j < file->hsBlobCount; j++)
					SAFE_RELEASE(file->hsBlobs[j].hs);
				allocs->dealloc(file->hsBlobs);
			}
			if (file->dsBlobs)
			{
				for (uint j = 0; j < file->dsBlobCount; j++)
					SAFE_RELEASE(file->dsBlobs[j].ds);
				allocs->dealloc(file->dsBlobs);
			}
		}

		allocs->dealloc(res->shaderFiles);
	}

	return S_OK;
}

int BitSizeOfFormatElement(DXGI_FORMAT format);
int ByteSizeOfFormatElement(DXGI_FORMAT format);
bool EqualInputElementDesc(int descCount, D3D11_INPUT_ELEMENT_DESC* descArray0, D3D11_INPUT_ELEMENT_DESC* descArray1);

HRESULT LoadGeometryFromFBXChunk(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device,
	int chunkCount, FBXChunk* chunks
)
{
	int startGeometryCount = res->geometryCount,
		startLayoutCount = res->vertexLayoutCount;
	for (int ci = 0; ci < chunkCount; ci++)
		for (int mi = 0; mi < chunks[ci].meshCount; mi++)
			res->geometryCount++;

	if (res->geometryCount != startGeometryCount)
	{
		res->geometryChunks =
			(DX11Resources::DX11GeometryChunk*)allocs->realloc(
				res->geometryChunks,
				sizeof(DX11Resources::DX11GeometryChunk) * res->geometryCount
			);
		memset(
			res->geometryChunks + startGeometryCount,
			0,
			sizeof(DX11Resources::DX11GeometryChunk) * (res->geometryCount - startGeometryCount)
		);
	}

	int vertexLayoutBufferCount = 0;
	DX11Resources::DX11LayoutChunk* vertexLayoutBuffer =
		(DX11Resources::DX11LayoutChunk*)alloca(
			sizeof(DX11Resources::DX11LayoutChunk) * (res->geometryCount - startGeometryCount)
		);
	const int descBufferCapacity = 32;
	D3D11_INPUT_ELEMENT_DESC* descBuffer = (D3D11_INPUT_ELEMENT_DESC*)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity);
	memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity);
	int vertexTempBufferSize = 0;
	byte* vertexTempBuffer = nullptr;

	for (int ci = 0, geometryOffset = startGeometryCount; ci < chunkCount; ci++, memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity))
	{
		FBXChunk& c = chunks[ci];

		for (int mi = 0; mi < c.meshCount; mi++, memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity))
		{
			// vertexlayout record start 
			FBXMeshChunk& m = c.meshs[mi];
			int accumulatedOffset = 0;

			int descCount = 0;
			descBuffer[descCount].SemanticName = "POSITION";
			descBuffer[descCount].SemanticIndex = 0;
			descBuffer[descCount].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			descBuffer[descCount].InputSlot = 0;
			descBuffer[descCount].AlignedByteOffset = accumulatedOffset;
			descBuffer[descCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			descBuffer[descCount].InstanceDataStepRate = 0;

			accumulatedOffset += ByteSizeOfFormatElement(descBuffer[descCount].Format);
			descCount++;

			if (m.geometry.normals)
			{
				descBuffer[descCount].SemanticName = "NORMAL";
				descBuffer[descCount].SemanticIndex = 0;
				descBuffer[descCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				descBuffer[descCount].InputSlot = 0;
				descBuffer[descCount].AlignedByteOffset = accumulatedOffset;
				descBuffer[descCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				descBuffer[descCount].InstanceDataStepRate = 0;

				accumulatedOffset += ByteSizeOfFormatElement(descBuffer[descCount].Format);
				descCount++;
			}
			if (m.geometry.tangents)
			{
				descBuffer[descCount].SemanticName = "TANGENT";
				descBuffer[descCount].SemanticIndex = 0;
				descBuffer[descCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				descBuffer[descCount].InputSlot = 0;
				descBuffer[descCount].AlignedByteOffset = accumulatedOffset;
				descBuffer[descCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				descBuffer[descCount].InstanceDataStepRate = 0;

				accumulatedOffset += ByteSizeOfFormatElement(descBuffer[descCount].Format);
				descCount++;
			}
			if (m.geometry.binormals)
			{
				descBuffer[descCount].SemanticName = "BINORMAL";
				descBuffer[descCount].SemanticIndex = 0;
				descBuffer[descCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				descBuffer[descCount].InputSlot = 0;
				descBuffer[descCount].AlignedByteOffset = accumulatedOffset;
				descBuffer[descCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				descBuffer[descCount].InstanceDataStepRate = 0;

				accumulatedOffset += ByteSizeOfFormatElement(descBuffer[descCount].Format);
				descCount++;
			}
			if (m.geometry.uvSlots && m.geometry.uvSlotCount > 0)
			{
				for (int uvi = 0; uvi < m.geometry.uvSlotCount; uvi++)
				{
					if (m.geometry.uvSlots[uvi])
					{
						descBuffer[descCount].SemanticName = "TEXCOORD";
						descBuffer[descCount].SemanticIndex = uvi;
						descBuffer[descCount].Format = DXGI_FORMAT_R32G32_FLOAT;
						descBuffer[descCount].InputSlot = 0;
						descBuffer[descCount].AlignedByteOffset = accumulatedOffset;
						descBuffer[descCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
						descBuffer[descCount].InstanceDataStepRate = 0;

						accumulatedOffset += ByteSizeOfFormatElement(descBuffer[descCount].Format);
						descCount++;
					}
				}
			}

			int findVertexLayoutIndex = -1, vertexSize;
			bool findVertexLayout = false;
			{
				for (int vli = 0; vli < res->vertexLayoutCount; vli++)
				{
					if (
						res->vertexLayouts[vli].descCount == descCount &&
						EqualInputElementDesc(descCount, descBuffer, res->vertexLayouts[vli].descs)
						)
					{
						findVertexLayout = true;
						findVertexLayoutIndex = vli;
						vertexSize = res->vertexLayouts[vli].vertexSize;
						break;
					}
				}

				if (!findVertexLayout)
					for (int vli = 0; vli < vertexLayoutBufferCount; vli++)
					{
						if (
							vertexLayoutBuffer[vli].descCount == descCount &&
							EqualInputElementDesc(descCount, descBuffer, vertexLayoutBuffer[vli].descs)
							)
						{
							findVertexLayout = true;
							findVertexLayoutIndex = vli + res->vertexLayoutCount;
							vertexSize = vertexLayoutBuffer[vli].vertexSize;
							break;
						}
					}
			}

			if (!findVertexLayout)
			{
				vertexLayoutBuffer[vertexLayoutBufferCount].vertexSize = vertexSize = accumulatedOffset;
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
			DX11Resources::DX11LayoutChunk& lc = res->vertexLayouts[findVertexLayoutIndex];
			DX11Resources::DX11GeometryChunk& g = res->geometryChunks[geometryOffset];
			g.vertexLayoutIndex = findVertexLayoutIndex;
			g.indexCount = m.geometry.indexCount;
			g.vertexCount = m.geometry.vertexCount;

			vertexTempBufferSize = vertexSize * g.vertexCount;
			vertexTempBuffer = (byte*)allocs->realloc(vertexTempBuffer, vertexTempBufferSize);

			byte* ptr = vertexTempBuffer;

			for (int i = 0; i < g.vertexCount; i++)
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
				for (int j = 0; j < m.geometry.uvSlotCount; j++)
					if (m.geometry.uvSlots[j])
					{
						memcpy(ptr, m.geometry.uvSlots[j] + i, sizeof(m.geometry.uvSlots[j][i]));
						ptr += sizeof(m.geometry.uvSlots[j][i]);
					}
			}

			FAILED_ERROR_MESSAGE_RETURN(
				CreateVertexBufferInline(
					device, &g.vertexBuffer, 
					vertexSize, g.vertexCount, vertexTempBuffer
				), 
				L"fail to create d3d11 vertex buffer.."
			);
			FAILED_ERROR_MESSAGE_RETURN(
				CreateIndexBufferInline(
					device, &g.indexBuffer, 
					sizeof(m.geometry.indices[0]), m.geometry.indexCount, m.geometry.indices
				),
				L"fail to create d3d11 index buffer.."
			);
			// geometry create end

			geometryOffset++;
		}
	}

	SAFE_DEALLOC(vertexTempBuffer, allocs->dealloc);

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

HRESULT LoadTexture2DAndSRVFromDirectories(DX11Resources* res, const Allocaters* allocs, ID3D11Device* device, int dirCount, const wchar_t** dirs, uint textureBufferSize, void* allocatedtextureBuffer)
{
	ASSERT(dirs != nullptr);

	size_t currentTextureBufferSize = textureBufferSize ? textureBufferSize : 2024 * 2024 * 4 * 4;
	void* textureBuffer = allocatedtextureBuffer? allocatedtextureBuffer: allocs->alloc(currentTextureBufferSize);
	int textureBufferArrayCapacity = dirCount;

	res->texture2Ds = (DX11Resources::DX11Texture2D*)allocs->realloc(res->texture2Ds, sizeof(DX11Resources::DX11Texture2D) * (textureBufferArrayCapacity + res->texture2DCount));
	memset(res->texture2Ds + res->texture2DCount, 0, sizeof(DX11Resources::DX11Texture2D) * textureBufferArrayCapacity);

	res->srvs = (ID3D11ShaderResourceView**)allocs->realloc(res->srvs, sizeof(ID3D11ShaderResourceView*) * dirCount);
	memset(res->srvs + res->srvCount, 0, sizeof(ID3D11ShaderResourceView*) * dirCount);

	FILE* fp; size_t size;
	for (int i = 0; i < dirCount; i++)
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

		FAILED_WARN_MESSAGE_CONTINUE(
			CreateShaderResourceViewInline(
				device,
				res->texture2Ds[i + res->texture2DCount].texture,
				&res->srvs[res->srvCount + i]
			),
			L"fail to create d3d11 srv object.."
		);
		res->texture2Ds[i + res->texture2DCount].srvIndex = res->srvCount + i;
	}

	res->srvCount += dirCount;
	res->texture2DCount += dirCount;
	SAFE_DEALLOC(textureBuffer, allocs->dealloc);

	return S_OK;
}

// TODO:: Append
HRESULT LoadShaderFromDirectoriesAndInputLayout(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device, 
	int compileCount, const DX11ShaderCompileDesc* descs, 
	int inputLayoutCount, const DX11InputLayoutDesc* inputLayoutDesc
)
{
	struct DescToFileShader
	{
		uint fileIndex;
		uint shaderKindIndex;
		uint shaderIndex;
	};
	std::vector<DescToFileShader> descToFileShader;
	std::vector<DX11Resources::DX11ShaderFile> files;
	for (int i = 0; i < compileCount; i++)
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

			int len = wcslen(dsc.fileName);
			wchar_t* tempFileName = (wchar_t*)allocs->alloc(sizeof(wchar_t) * (len + 1));
			wcscpy_s(tempFileName, (len + 1), dsc.fileName);
			file.fileName = tempFileName;

			fileIndex = files.size();
			files.push_back(file);
		}

		DX11Resources::DX11ShaderFile& file = files[fileIndex];
		uint shaderKindIndex = 0;
		if (!ShaderTargetToIndex(dsc.target[0], &shaderKindIndex))
			ERROR_MESSAGE_CONTINUE_ARGS(L"corruption of shader desc : (%s, %s, [%c]%s)", dsc.fileName, dsc.entrypoint, dsc.target[0], dsc.target + 1);

		uint* count = &file.shadersByKind[shaderKindIndex].shaderCount;
		DX11Resources::DX11ShaderFile::DX11CompiledShader** compiledShaders = 
			&file.shadersByKind[shaderKindIndex].shaders;

		*compiledShaders =
			(DX11Resources::DX11ShaderFile::DX11CompiledShader*)allocs->realloc(
				*compiledShaders,
				sizeof(DX11Resources::DX11ShaderFile::DX11CompiledShader) * (*count + 1)
			);

		(*compiledShaders)[*count].entrypoint = (char*)allocs->alloc(sizeof(char)*(strlen(dsc.entrypoint) + 1));
		strcpy_s((*compiledShaders)[*count].entrypoint, (strlen(dsc.entrypoint) + 1), dsc.entrypoint);
		(*compiledShaders)[*count].target = (char*)allocs->alloc(sizeof(char)*(strlen(dsc.target) + 1));
		strcpy_s((*compiledShaders)[*count].target, (strlen(dsc.target) + 1), dsc.target);

		descToFileShader.push_back({ (uint)fileIndex, shaderKindIndex, *count });

		(*count)++;
	}

	int currentFileCount = 0, prevShaderFileCount = res->shaderFileCount;
	res->shaderFileCount = prevShaderFileCount + files.size();
	res->shaderFiles =
		(DX11Resources::DX11ShaderFile*)allocs->realloc(
			res->shaderFiles,
			sizeof(DX11Resources::DX11ShaderFile) * (prevShaderFileCount + files.size())
		);

	memcpy(res->shaderFiles + prevShaderFileCount, files.data(), sizeof(DX11Resources::DX11ShaderFile) * res->shaderFileCount);

	res->inputLayoutCount = inputLayoutCount;
	res->inputLayouts = 
		(DX11Resources::DX11InputLayout*)allocs->alloc(
			sizeof(DX11Resources::DX11InputLayout) * inputLayoutCount
		);
	memset(res->inputLayouts, 0, sizeof(DX11Resources::DX11InputLayout) * inputLayoutCount);
	for (int i = 0; i < compileCount; i++)
	{
		const DX11ShaderCompileDesc& dsc = descs[i];
		const DescToFileShader& dtof = descToFileShader[i];

		DX11Resources::DX11ShaderFile::DX11CompiledShader* compiledShader = 
			&res->shaderFiles[dtof.fileIndex + prevShaderFileCount].shadersByKind[dtof.shaderKindIndex].shaders[dtof.shaderIndex];

		bool isDebug =
#if defined (_DEBUG) || (DEBUG)
			true
#else
			false
#endif
			;
		ID3DBlob* blob;
		FAILED_ERROR_MESSAGE_CONTINUE(
			CompileShaderFromFile(dsc.fileName, dsc.entrypoint, dsc.target, isDebug, &blob),
			L"fail to compile shader.."
		);

		HRESULT hr;
		switch (dsc.target[0])
		{
		case 'v':
			hr = device->CreateVertexShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &compiledShader->vs
			);
			break;
		case 'p':
			hr = device->CreatePixelShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &compiledShader->ps
			);
			break;
		case 'c':
			hr = device->CreateComputeShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &compiledShader->cs
			);
			break;
		case 'g':
			hr = device->CreateGeometryShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &compiledShader->gs
			);
			break;
		case 'h':
			hr = device->CreateHullShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &compiledShader->hs
			);
			break;
		case 'd':
			hr = device->CreateDomainShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &compiledShader->ds
			);
			break;
		default:
			ERROR_MESSAGE_CONTINUE_ARGS(L"corruption of shader desc : (%s, %s, [%c]%s)", dsc.fileName, dsc.entrypoint, dsc.target[0], dsc.target + 1);
		}
		FAILED_ERROR_MESSAGE_CONTINUE(hr, L"fail to create shader..");

		if (dsc.target[0] == 'v')
			for (int inputLayoutIndex = 0; inputLayoutIndex < inputLayoutCount; inputLayoutIndex++)
			{
				if (
					res->inputLayouts[inputLayoutIndex].inputLayout == nullptr &&
					inputLayoutDesc[inputLayoutIndex].shaderCompileDescIndex == i
					)
				{
					const auto& vertexLayoutDesc = 
						res->vertexLayouts[inputLayoutDesc[inputLayoutIndex].layoutChunkIndex];
					auto& inputLayoutChunk = res->inputLayouts[inputLayoutIndex];
					inputLayoutChunk.vertexShaderIndex = dtof.shaderIndex;
					inputLayoutChunk.shaderFileIndex = dtof.fileIndex;
					inputLayoutChunk.layoutChunkIndex = inputLayoutDesc[inputLayoutIndex].layoutChunkIndex;

					hr = device->CreateInputLayout(
						vertexLayoutDesc.descs, vertexLayoutDesc.descCount,
						blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayoutChunk.inputLayout
					);
					FAILED_ERROR_MESSAGE_ARGS(hr, L"fail to create input layout..");
					break;
				}
			}

		blob->Release();
	}

	return S_OK;
}

HRESULT CreateSamplerStates(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device,
	int samplerCount, D3D11_SAMPLER_DESC* descs
)
{
	res->samplerStates = (ID3D11SamplerState**)allocs->realloc(res->samplerStates, sizeof(void*) * (res->samplerCount + samplerCount));

	HRESULT hr = S_OK;

	for (int i = res->samplerCount; i < samplerCount; i++)
	{
		hr = device->CreateSamplerState(descs + i, res->samplerStates + res->samplerCount + i);
		FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create sampler state..");
	}

	res->samplerCount += samplerCount;

	return hr;
}

HRESULT CreateConstantBuffers(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device,
	int constantBufferCount, const size_t* bufferSizes
)
{
	res->constantBuffers = (ID3D11Buffer**)realloc(res->constantBuffers, sizeof(ID3D11Buffer*) * (res->constantBufferCount + constantBufferCount));

	for (int i = 0; i < constantBufferCount; i++)
	{
		FAILED_ERROR_MESSAGE(
			CreateConstantBufferInline(device, &res->constantBuffers[i + res->constantBufferCount], bufferSizes[i]),
			L"fail to create on frame constant buffer.."
		);
	}

	res->constantBufferCount += constantBufferCount;
		
	return S_OK;
}
HRESULT UploadConstantBuffer(
	DX11Resources* res, ID3D11DeviceContext* deviceContext, int constantBufferIndex, void* uploadData
)
{
	HRESULT hr = S_OK;
	deviceContext->UpdateSubresource(res->constantBuffers[constantBufferIndex], 0, nullptr, uploadData, 0, 0);
	return hr;
}
HRESULT DrawingContextStatePrepare(DX11ContextState* state, const Allocaters* allocs, int dependCount, const DX11PipelineDependancy* depends)
{
	int maxCount = state->bufferCount;
	for (int i = 0; i < dependCount; i++)
	{
		const DX11PipelineDependancy& depend = depends[i];
		for (int k = 0; k < 6; k++)
		{
			DX11ShaderResourceDependancy shaderDep = depend.dependants[k];

			for (int j = 0; j < shaderDep.constantBufferCount; j++)
			{
				auto ref = shaderDep.constantBuffers[j];
				maxCount = max(ref.indexCount, maxCount);
			}
			for (int j = 0; j < shaderDep.samplerCount; j++)
			{
				auto ref = shaderDep.samplers[j];
				maxCount = max(ref.indexCount, maxCount);
			}
			for (int j = 0; j < shaderDep.srvCount; j++)
			{
				auto ref = shaderDep.srvs[j];
				maxCount = max(ref.indexCount, maxCount);
			}
			for (int j = 0; j < shaderDep.uavCount; j++)
			{
				auto ref = shaderDep.uavs[j];
				maxCount = max(ref.indexCount, maxCount);
			}
		}
	}

	if (state->bufferCount < maxCount)
	{
		state->bufferPtrBuffer = (void**)allocs->realloc(state->bufferPtrBuffer, sizeof(void*) * maxCount);
		state->numberBuffer = (uint*)allocs->realloc(state->numberBuffer, sizeof(uint) * maxCount);
	}
		
	return S_OK;
}
HRESULT DrawExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, int dependCount, const DX11PipelineDependancy* depends)
{
	uint offfset = 0;
	for (int i = 0; i < dependCount; i++)
	{
		const DX11PipelineDependancy& depend = depends[i];
		const auto& depIn = depend.input;
		const auto& resGeo = res->geometryChunks[depIn.geometryIndex];

		deviceContext->IASetInputLayout(res->inputLayouts[depIn.inputLayoutIndex].inputLayout);
		deviceContext->IASetVertexBuffers(0, 1, &resGeo.vertexBuffer, &res->vertexLayouts[resGeo.vertexLayoutIndex].vertexSize, &depIn.vertexBufferOffset);
		deviceContext->IASetIndexBuffer(resGeo.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(depIn.topology);

		if (depend.vs.shaderFileIndex >= 0)
		{
			deviceContext->VSSetShader(res->shaderFiles[depend.vs.shaderFileIndex].vsBlobs[depend.vs.shaderIndex].vs, nullptr, 0);

			for (int j = 0; j < depend.vs.constantBufferCount; j++)
			{
				auto ref = depend.vs.constantBuffers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->constantBuffers[ref.indices[k]];
				deviceContext->VSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.vs.samplerCount; j++)
			{
				auto ref = depend.vs.samplers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->VSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.vs.srvCount; j++)
			{
				auto ref = depend.vs.srvs[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->VSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.ps.shaderFileIndex >= 0)
		{
			deviceContext->PSSetShader(res->shaderFiles[depend.ps.shaderFileIndex].psBlobs[depend.ps.shaderIndex].ps, nullptr, 0);

			for (int j = 0; j < depend.ps.constantBufferCount; j++)
			{
				auto ref = depend.ps.constantBuffers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->constantBuffers[ref.indices[k]];
				deviceContext->PSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.ps.samplerCount; j++)
			{
				auto ref = depend.ps.samplers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->PSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.ps.srvCount; j++)
			{
				auto ref = depend.ps.srvs[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->PSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.cs.shaderFileIndex >= 0)
		{
			deviceContext->CSSetShader(res->shaderFiles[depend.cs.shaderFileIndex].csBlobs[depend.cs.shaderIndex].cs, nullptr, 0);

			for (int j = 0; j < depend.cs.constantBufferCount; j++)
			{
				auto ref = depend.cs.constantBuffers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->constantBuffers[ref.indices[k]];
				deviceContext->CSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.cs.samplerCount; j++)
			{
				auto ref = depend.cs.samplers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->CSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.cs.srvCount; j++)
			{
				auto ref = depend.cs.srvs[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->CSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.cs.uavCount; j++)
			{
				auto ref = depend.cs.uavs[j];
				for (int k = 0; k < ref.indexCount; k++)
				{
					// TODO:: set uav initial count
					state->numberBuffer[k] = 0;
					state->bufferPtrBuffer[k] = res->uavs[ref.indices[k]];
				}					
				deviceContext->CSSetUnorderedAccessViews(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11UnorderedAccessView *const *>(state->bufferPtrBuffer), state->numberBuffer);
			}
		}

		if (depend.gs.shaderFileIndex >= 0)
		{
			deviceContext->GSSetShader(res->shaderFiles[depend.gs.shaderFileIndex].gsBlobs[depend.gs.shaderIndex].gs, nullptr, 0);

			for (int j = 0; j < depend.gs.constantBufferCount; j++)
			{
				auto ref = depend.gs.constantBuffers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->constantBuffers[ref.indices[k]];
				deviceContext->GSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.gs.samplerCount; j++)
			{
				auto ref = depend.gs.samplers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->GSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.gs.srvCount; j++)
			{
				auto ref = depend.gs.srvs[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->GSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.hs.shaderFileIndex >= 0)
		{
			deviceContext->HSSetShader(res->shaderFiles[depend.hs.shaderFileIndex].hsBlobs[depend.hs.shaderIndex].hs, nullptr, 0);

			for (int j = 0; j < depend.hs.constantBufferCount; j++)
			{
				auto ref = depend.hs.constantBuffers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->constantBuffers[ref.indices[k]];
				deviceContext->HSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.hs.samplerCount; j++)
			{
				auto ref = depend.hs.samplers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->HSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.hs.srvCount; j++)
			{
				auto ref = depend.hs.srvs[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->HSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.ds.shaderFileIndex >= 0)
		{
			deviceContext->DSSetShader(res->shaderFiles[depend.ds.shaderFileIndex].dsBlobs[depend.ds.shaderIndex].ds, nullptr, 0);

			for (int j = 0; j < depend.ds.constantBufferCount; j++)
			{
				auto ref = depend.ds.constantBuffers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->constantBuffers[ref.indices[k]];
				deviceContext->DSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.ds.samplerCount; j++)
			{
				auto ref = depend.ds.samplers[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->samplerStates[ref.indices[k]];
				deviceContext->DSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (int j = 0; j < depend.ds.srvCount; j++)
			{
				auto ref = depend.ds.srvs[j];
				for (int k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = res->srvs[ref.indices[k]];
				deviceContext->DSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		switch(depend.drawType)
		{
		case DX11PipelineDependancy::DrawType::DRAW:
			deviceContext->Draw(
				depend.argsAsDraw.drawArgs.vertexCount, depend.argsAsDraw.drawArgs.startVertexLocation
			);
			break;
		case DX11PipelineDependancy::DrawType::DRAW_INDEXED:
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
HRESULT DrawImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, int dependCount, const DX11PipelineDependancy* depends)
{
	return E_NOTIMPL;
}
HRESULT ReleaseDependancy(DX11PipelineDependancy* dependancy, const Allocaters* allocs)
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
