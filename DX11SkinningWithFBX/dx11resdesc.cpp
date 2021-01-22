#include "dx11resdesc.h"

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
void ReserveLoadShaders(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const ShaderCompileDesc* descs, OUT ShaderKind* kinds, OUT int* indices)
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
uint ReserveLoadShader(DX11InternalResourceDescBuffer* rawResBuffer, const ShaderCompileDesc* desc, OUT ShaderKind* s)
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