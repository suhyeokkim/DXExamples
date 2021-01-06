#include "dx11res.h"

#include <DirectXTex.h>
#include <vector>
#include <d3d11_4.h>
#include <pix3.h>
#include <array>

#include "defined.h"
#include "dx11util.h"


HRESULT ReserveLoadInputLayout(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
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

		res->inputLayouts[i].inputLayoutIndex = ReserveLoadInputLayout(rawBuffer, desc);
		res->inputLayouts[i].vertexShaderIndex = dtof.shaderIndex;
		res->inputLayouts[i].layoutChunkIndex = d.layoutChunkIndex;
	}

	return S_OK;
}

uint ReserveLoadInputLayouts(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11ILDesc* descs)
{
	rawResBuffer->inputLayoutDescs.insert(rawResBuffer->inputLayoutDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->inputLayoutDescs.size() - additioanlCount;
}
uint ReserveLoadConstantBuffers(DX11InternalResourceDescBuffer* rawResBuffer, uint constantBufferCount, const uint* bufferSizes)
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
void ReserveLoadShaders(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11ShaderCompileDesc* descs, OUT ShaderKind* kinds, OUT int* indices)
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
uint ReserveLoadSamplerStates(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const D3D11_SAMPLER_DESC* descs)
{
	rawResBuffer->samplerDescs.insert(rawResBuffer->samplerDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->samplerDescs.size() - additioanlCount;
}
uint ReserveLoadShaderResourceViews(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11SRVDesc* descs)
{
	rawResBuffer->srvDescs.insert(rawResBuffer->srvDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->srvDescs.size() - additioanlCount;
}
uint ReserveLoadUnorderedAccessViews(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11UAVDesc* descs)
{
	rawResBuffer->uavDescs.insert(rawResBuffer->uavDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->uavDescs.size() - additioanlCount;
}
uint ReserveLoadBuffers(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11BufferDesc* descs)
{
	rawResBuffer->bufferDescs.insert(rawResBuffer->bufferDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->bufferDescs.size() - additioanlCount;
}
uint ReserveLoadTexture2Ds(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11Texture2DDesc* descs)
{
	rawResBuffer->tex2DDescs.insert(rawResBuffer->tex2DDescs.end(), descs, descs + additioanlCount);
	return (uint)rawResBuffer->tex2DDescs.size() - additioanlCount;
}

uint ReserveLoadInputLayout(DX11InternalResourceDescBuffer* rawResBuffer, const DX11ILDesc& desc)
{
	rawResBuffer->inputLayoutDescs.push_back(desc);
	return (uint)rawResBuffer->inputLayoutDescs.size() - 1;
}
uint ReserveLoadConstantBuffer(DX11InternalResourceDescBuffer* rawResBuffer, const uint bufferSize)
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
uint ReserveLoadSamplerState(DX11InternalResourceDescBuffer* rawResBuffer, const D3D11_SAMPLER_DESC* desc)
{
	rawResBuffer->samplerDescs.push_back(*desc);
	return (uint)rawResBuffer->samplerDescs.size() - 1;
}
uint ReserveLoadShader(DX11InternalResourceDescBuffer* rawResBuffer, const DX11ShaderCompileDesc* desc, OUT ShaderKind* s)
{
	int index = ShaderTargetToIndex(desc->target[0]);
	FALSE_ERROR_MESSAGE_RETURN_CODE(index >= 0, L"fail to identify shader by target..", UINT_MAX);
	if (s) *s = (ShaderKind)index;
	rawResBuffer->shaderCompileDesces[index].push_back(*desc);
	return (uint)rawResBuffer->shaderCompileDesces[index].size() - 1;
}
uint ReserveLoadShaderResourceView(DX11InternalResourceDescBuffer* rawResBuffer, const DX11SRVDesc* desc)
{
	rawResBuffer->srvDescs.push_back(*desc);
	return (uint)rawResBuffer->srvDescs.size() - 1;
}
uint ReserveLoadUnorderedAccessView(DX11InternalResourceDescBuffer* rawResBuffer, const DX11UAVDesc* desc)
{
	rawResBuffer->uavDescs.push_back(*desc);
	return (uint)rawResBuffer->uavDescs.size() - 1;
}
uint ReserveLoadBuffer(DX11InternalResourceDescBuffer* rawResBuffer, const DX11BufferDesc* desc)
{
	rawResBuffer->bufferDescs.push_back(*desc);
	return (uint)rawResBuffer->bufferDescs.size() - 1;
}
uint ReserveLoadTexture2D(DX11InternalResourceDescBuffer* rawResBuffer, const DX11Texture2DDesc* desc)
{
	rawResBuffer->tex2DDescs.push_back(*desc);
	return (uint)rawResBuffer->tex2DDescs.size() - 1;
}

HRESULT CreateDX11ResourcesByDesc(DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device, bool isDebug)
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

	size_t prevSize = 0;
	void* ptr = nullptr;

	REALLOC_RANGE_ZEROMEM(
		prevTex2DCount, res->texture2DCount, rawBuffer->tex2DDescs.size(),
		ID3D11Texture2D*, res->texture2Ds, allocs->realloc
	);
	for (uint i = prevTex2DCount; i < res->texture2DCount; i++)
	{
		DX11Texture2DDesc& desc = rawBuffer->tex2DDescs[i - prevTex2DCount];

		if (desc.loadFromFile)
		{
			if (!desc.fileName)
				ERROR_MESSAGE_CONTINUE_ARGS(L"fail to load texture because fileName is null..(idx:%d)", i);

			FILE* fp;
			_wfopen_s(&fp, desc.fileName, L"rb");
			size_t size = GetFileSize(fp);
			if (prevSize < size)
			{
				prevSize = size;
				ptr = allocs->realloc(ptr, prevSize);
			}
			size_t readSize = fread(ptr, 1, size, fp);
			fclose(fp);
			fp = nullptr;

			if (readSize != size)
				continue;

			DirectX::ScratchImage img;
			FAILED_WARN_MESSAGE_CONTINUE_ARGS(
				DirectX::LoadFromWICMemory(ptr, size, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, nullptr, img),
				L"fail to load image from file:\"%s\"..",
				desc.fileName
			);

			FAILED_WARN_MESSAGE_CONTINUE(
				DirectX::CreateTextureEx(
					device,
					img.GetImages(), img.GetImageCount(), img.GetMetadata(),
					desc.usage, desc.bindFlags, desc.cpuAccessFlags, desc.miscFlags, desc.forceSRGB,
					reinterpret_cast<ID3D11Resource**>(&res->texture2Ds[i])
				),
				L"fail to CreateTextureEx().."
			);
		}
		else
		{
			FAILED_WARN_MESSAGE_CONTINUE(
				device->CreateTexture2D(&desc.tex2DDesc, desc.subres.pSysMem? &desc.subres: nullptr, &res->texture2Ds[i]),
				L"fail to create d3d11 texture2d object.."
			);
		}
	}

	REALLOC_RANGE_ZEROMEM(
		prevBufferCount, res->bufferCount, rawBuffer->bufferDescs.size(),
		ID3D11Buffer*, res->buffers, allocs->realloc
	);
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

		// TODO:: 텍스쳐 로드 확인 용 스핀락, 비동기 로드시 고쳐야함
		if ((bool)rawBuffer->srvDescs[i].setSRVDesc)
			while (!rawBuffer->srvDescs[i].setSRVDesc(&rawBuffer->srvDescs[i].view));

		switch (rawBuffer->srvDescs[i].view.ViewDimension)
		{
		case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER:
			dx11res = res->buffers[rawBuffer->srvDescs[i].bufferIndex];
			break;
		case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D:
			dx11res = res->texture2Ds[rawBuffer->srvDescs[i].texture2DIndex];
			break;
		default:
			ERROR_MESSAGE_CONTINUE_ARGS(
				L"failed by corrupted view type:%d\n",
				rawBuffer->srvDescs[i].view.ViewDimension
			);
			break;
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

HRESULT UploadDX11ConstantBuffer(
	DX11Resources* res, ID3D11DeviceContext* deviceContext, uint constantBufferIndex, void* uploadData
)
{
	HRESULT hr = S_OK;
	deviceContext->UpdateSubresource(res->buffers[res->constantBufferIndices[constantBufferIndex]], 0, nullptr, uploadData, 0, 0);
	return hr;
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

int BitSizeOfFormatElement(DXGI_FORMAT format);
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
