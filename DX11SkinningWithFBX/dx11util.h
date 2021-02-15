#pragma once

#include <vector>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include "fbximport.h"

inline size_t GetFileSize(FILE* fp)
{
	size_t size;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

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

inline HRESULT GetDXGIAdaptersInline(IDXGIFactory1* factory, int* adapterCount, IDXGIAdapter1** dxgiAdapterArray)
{
	eastl::vector<IDXGIAdapter1*, EASTLAllocator> dxgiAdapters(EASTL_TEMPARARY_NAME);
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

bool EqualInputElementDesc(int descCount, D3D11_INPUT_ELEMENT_DESC* descArray0, D3D11_INPUT_ELEMENT_DESC* descArray1);
int ByteSizeOfFormatElement(DXGI_FORMAT format);
int BitSizeOfFormatElement(DXGI_FORMAT format);