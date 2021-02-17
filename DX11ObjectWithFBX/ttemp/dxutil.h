#pragma once

#include <vector>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include "fbximport.h"

inline HRESULT CompileShaderFromFile(IN const wchar_t* fileName, IN const char* entryPoint, IN const char* sm, IN bool debug, OUT ID3DBlob** outBlob)
{
	HRESULT hr = S_OK;

	// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/d3dcompile-constants
	UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	if (debug)
		shaderFlags |= D3DCOMPILE_DEBUG;

	ID3DBlob* errorBlob;
	hr = D3DCompileFromFile(
		fileName, nullptr, nullptr, entryPoint, sm,
		shaderFlags, 0, outBlob, &errorBlob
	);
	if (FAILED(hr))
	{
		if (errorBlob != NULL)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			fputs((char*)errorBlob->GetBufferPointer(), stderr);
		}
		if (errorBlob) errorBlob->Release();
		return hr;
	}
	//else
	//{
	//	ID3DBlob* disassembled;
	//	D3DDisassemble((*outBlob)->GetBufferPointer(), (*outBlob)->GetBufferSize(), 0, "", &disassembled);
	//	char disassembledText[4096];
	//	memcpy(disassembled->GetBufferPointer(), disassembledText, disassembled->GetBufferSize());
	//	disassembledText[disassembled->GetBufferSize()] = '\0';
	//	fputs(disassembledText, stdout);
	//	disassembled->Release();
	//}
	if (errorBlob) errorBlob->Release();

	return hr;
}


inline HRESULT CreateVertexBufferInline(ID3D11Device* device, ID3D11Buffer** vertexBuffer, UINT vertexSize, UINT vertexCount, void* vertices)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = vertexSize * vertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subrscData;
	memset(&subrscData, 0, sizeof(subrscData));
	subrscData.pSysMem = vertices;
	hr = device->CreateBuffer(&bufferDesc, &subrscData, vertexBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create vertex buffer..");

	return hr;
}

inline HRESULT CreateIndexBufferInline(ID3D11Device* device, ID3D11Buffer** indexBuffer, UINT indexSize, UINT indexCount, void* indices)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = indexSize * indexCount;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subrscData;
	memset(&subrscData, 0, sizeof(subrscData));
	subrscData.pSysMem = indices;
	hr = device->CreateBuffer(&bufferDesc, &subrscData, indexBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create vertex buffer..");

	return hr;
}

inline HRESULT CreateIndexBufferInline(ID3D11Device* device, ID3D11Buffer** indexBuffer, UINT indexSize, UINT primSize, UINT primCount, void* indices)
{
	return CreateIndexBufferInline(device, indexBuffer, indexSize, primSize * primCount, indices);
}

inline HRESULT CreateSampler(ID3D11Device* device, ID3D11SamplerState** sampler)
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;
	hr = device->CreateSamplerState(&samplerDesc, sampler);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create sampler state..");

	return hr;
}

inline HRESULT CreateConstantBufferInline(ID3D11Device* device, ID3D11Buffer** constantBuffer, UINT size)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create constant buffer");

	return hr;
}

inline HRESULT GetDXGIAdaptersInline(IDXGIFactory1* factory, int* adapterCount, IDXGIAdapter1** dxgiAdapterArray)
{
	std::vector<IDXGIAdapter1*> dxgiAdapters;
	IDXGIAdapter1* dxgiAdapter;
	*adapterCount = 0;
	while (SUCCEEDED(factory->EnumAdapters1((*adapterCount)++, &dxgiAdapter)))
		dxgiAdapters.push_back(dxgiAdapter);
	*dxgiAdapterArray = (IDXGIAdapter1*)malloc(sizeof(IDXGIAdapter1*) * *adapterCount);
	memcpy(*dxgiAdapterArray, dxgiAdapters.data(), sizeof(IDXGIAdapter1*) * *adapterCount);

	return *adapterCount > 0 ? S_OK : E_FAIL;
}

inline HRESULT CreateSwapChainInline(HWND hWnd, IDXGIFactory* factory, ID3D11Device* device, IDXGISwapChain** swapChain, UINT width, UINT height, UINT maxFrameRate, bool isHDR10)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = isHDR10 ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = maxFrameRate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	return factory->CreateSwapChain(device, &swapChainDesc, swapChain);
}

inline HRESULT CreateDepthStencilInline(ID3D11Device* device, ID3D11Texture2D** buffer, ID3D11DepthStencilView** view, UINT width, UINT height)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC descDepth;
	memset(&descDepth, 0, sizeof(D3D11_TEXTURE2D_DESC));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	hr = device->CreateTexture2D(&descDepth, nullptr, buffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create texture2d for depthstencil..");

	D3D11_DEPTH_STENCIL_VIEW_DESC descView;
	memset(&descView, 0, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descView.Format = descDepth.Format;
	descView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descView.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(*buffer, &descView, view);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create depthstencialview..");

	return hr;
}

inline HRESULT CreateShaderResourceViewInline(ID3D11Device* device, ID3D11Texture2D* res, ID3D11ShaderResourceView** srv)
{
	D3D11_TEXTURE2D_DESC texDesc;
	res->GetDesc(&texDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	memset(&desc, 0, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	desc.Format = texDesc.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;

	return device->CreateShaderResourceView(res, &desc, srv);
}

inline HRESULT CreateRenderTargetViewInline(ID3D11Device* device, ID3D11Texture2D* backBuffer, ID3D11RenderTargetView** rtv)
{
	return device->CreateRenderTargetView(backBuffer, nullptr, rtv);
}
