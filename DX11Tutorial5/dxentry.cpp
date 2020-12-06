#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

#include "dxentry.h"
#include "defined.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "usp10.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")

// windonw instances
HWND g_hWnd;
HINSTANCE g_hInstance;

// DXGI objects
IDXGIFactory7* g_DXGIFactory;
IDXGISwapChain* g_DXGISwapChain;

// D3D11 device objects
ID3D11Device* g_D3D11Device;
ID3D11DeviceContext* g_D3D11ImmediateContext;
ID3D11RenderTargetView* g_D3D11RenderTargetView;

// D3D shader resources
ID3D11Texture2D* g_D3D11DepthStencilTexture;
ID3D11DepthStencilView* g_D3D11DepthStencialView;

int g_ElementDescCount;
D3D11_INPUT_ELEMENT_DESC* g_D3D11InputElementDescArray;
ID3D11VertexShader* g_D3D11VertexShader;
ID3D11PixelShader* g_D3D11PixelShader;
ID3D11InputLayout* g_D3D11VertexLayout;
D3D11_VIEWPORT g_D3D11ViewPort;

UINT g_VertexCount, g_VertexSize;
ID3D11Buffer* g_D3D11VertexBuffer;
UINT g_PrimCount, g_PrimSize;
ID3D11Buffer* g_D3D11IndexBuffer;
ID3D11Buffer* g_D3D11ConstantBuffer;
XMMATRIX g_World, g_World2;
XMMATRIX g_View;
XMMATRIX g_Projection;

inline HRESULT GetDXGIAdaptersInline(IDXGIFactory1* factory, int* adapterCount, IDXGIAdapter1** dxgiAdapterArray)
{
	std::vector<IDXGIAdapter1*> dxgiAdapters;
	IDXGIAdapter1* dxgiAdapter;
	*adapterCount = 0;
	while (SUCCEEDED(factory->EnumAdapters1((*adapterCount)++, &dxgiAdapter)))
		dxgiAdapters.push_back(dxgiAdapter);
	*dxgiAdapterArray = (IDXGIAdapter1*)malloc(sizeof(IDXGIAdapter1*) * *adapterCount);
	memcpy(*dxgiAdapterArray, dxgiAdapters.data(), sizeof(IDXGIAdapter1*) * *adapterCount);

	return *adapterCount > 0? S_OK: E_FAIL;
}

inline HRESULT CreateSwapChainInline(IDXGIFactory* factory, ID3D11Device* device, IDXGISwapChain** swapChain, UINT width, UINT height, UINT maxFrameRate, bool isHDR10)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = isHDR10 ? DXGI_FORMAT_R10G10B10A2_UNORM: DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = maxFrameRate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = g_hWnd;
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
	FAILED_MESSAGE_RETURN(hr, "fail to create texture2d for depthstencil..");

	D3D11_DEPTH_STENCIL_VIEW_DESC descView;
	memset(&descView, 0, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descView.Format = descDepth.Format;
	descView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descView.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(*buffer, &descView, view);
	FAILED_MESSAGE_RETURN(hr, "fail to create depthstencialview..");
}

inline HRESULT CreateRenderTargetViewInline(ID3D11Device* device, ID3D11Texture2D* backBuffer, ID3D11RenderTargetView** rtv)
{
	return g_D3D11Device->CreateRenderTargetView(backBuffer, nullptr, &g_D3D11RenderTargetView);
}

HRESULT DXDeviceInit(UINT width, UINT height, UINT maxFrameRate, bool debug)
{
	HRESULT hr = S_OK;

	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&g_DXGIFactory));
	FAILED_MESSAGE_RETURN(hr, L"fail to create DXGIFactory2..");

	D3D_FEATURE_LEVEL maxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	UINT createDeviceFlags = 0;
	if (debug)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
		featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		&g_D3D11Device, &maxSupportedFeatureLevel, &g_D3D11ImmediateContext);
	FAILED_MESSAGE_RETURN(hr, L"fail to create D3D11Device..");

	hr = CreateSwapChainInline(g_DXGIFactory, g_D3D11Device, &g_DXGISwapChain, width, height, maxFrameRate, false);
	FAILED_MESSAGE_RETURN(hr, L"fail to create SwapChain..");

	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	FAILED_MESSAGE_RETURN(hr, L"fail to get buffer from swapchain..");

	hr = g_D3D11Device->CreateRenderTargetView(backBuffer, nullptr, &g_D3D11RenderTargetView);
	FAILED_MESSAGE_RETURN(hr, L"fail to create rendertargetview..");

	hr = CreateDepthStencilInline(g_D3D11Device, &g_D3D11DepthStencilTexture, &g_D3D11DepthStencialView, width, height);

	g_D3D11ViewPort.Width = (FLOAT)width;
	g_D3D11ViewPort.Height = (FLOAT)height;
	g_D3D11ViewPort.MinDepth = 0.0f;
	g_D3D11ViewPort.MaxDepth = 1.0f;
	g_D3D11ViewPort.TopLeftX = 0;
	g_D3D11ViewPort.TopLeftY = 0;
	
	return hr;
}

HRESULT CompileShaderFromFile(IN const wchar_t* fileName, IN const char* entryPoint, IN const char* sm, IN bool debug, OUT ID3DBlob** outBlob)
{
	HRESULT hr = S_OK;

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
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		if (errorBlob) errorBlob->Release();
		return hr;
	}
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
	FAILED_MESSAGE_RETURN(hr, L"fail to create vertex buffer");

	return hr;
}

inline HRESULT CreateIndexBufferInline(ID3D11Device* device, ID3D11Buffer** indexBuffer, UINT primSize, UINT primCount, void* indices)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = primSize * primCount;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subrscData;
	memset(&subrscData, 0, sizeof(subrscData));
	subrscData.pSysMem = indices;
	hr = device->CreateBuffer(&bufferDesc, &subrscData, indexBuffer);
	FAILED_MESSAGE_RETURN(hr, L"fail to create vertex buffer");

	return hr;
}

struct Tutorial4ConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};

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
	FAILED_MESSAGE_RETURN(hr, L"fail to create constant buffer");

	return hr;
}

HRESULT DXShaderResourceInit(const wchar_t* shaderFileName, bool debug)
{
	HRESULT hr = S_OK;

	ID3DBlob* VSBlob = nullptr;
	hr = CompileShaderFromFile(shaderFileName, "vertex", "vs_5_0", debug, &VSBlob);
	FAILED_MESSAGE_RETURN(hr, L"fail to compile vertex shader source..");

	hr = g_D3D11Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &g_D3D11VertexShader);
	FAILED_MESSAGE_RETURN(hr, L"fail to create vertex shader..");

	g_ElementDescCount = 2;
	g_D3D11InputElementDescArray = new D3D11_INPUT_ELEMENT_DESC[g_ElementDescCount];
	g_D3D11InputElementDescArray[0].SemanticName = "POSITION";
	g_D3D11InputElementDescArray[0].SemanticIndex = 0;
	g_D3D11InputElementDescArray[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	g_D3D11InputElementDescArray[0].InputSlot = 0;
	g_D3D11InputElementDescArray[0].AlignedByteOffset = 0;
	g_D3D11InputElementDescArray[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	g_D3D11InputElementDescArray[0].InstanceDataStepRate = 0;

	g_D3D11InputElementDescArray[1].SemanticName = "COLOR";
	g_D3D11InputElementDescArray[1].SemanticIndex = 0;
	g_D3D11InputElementDescArray[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	g_D3D11InputElementDescArray[1].InputSlot = 0;
	g_D3D11InputElementDescArray[1].AlignedByteOffset = 12;
	g_D3D11InputElementDescArray[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	g_D3D11InputElementDescArray[1].InstanceDataStepRate = 0;

	hr = g_D3D11Device->CreateInputLayout(
		g_D3D11InputElementDescArray, g_ElementDescCount,
		VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &g_D3D11VertexLayout
	);
	VSBlob->Release();
	FAILED_MESSAGE_RETURN(hr, L"fail to create input layout..");

	ID3DBlob* PSBlob = nullptr;
	hr = CompileShaderFromFile(shaderFileName, "pixel", "ps_5_0", debug, &PSBlob);
	FAILED_MESSAGE_RETURN(hr, L"fail to compile pixel shader source..");

	hr = g_D3D11Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &g_D3D11PixelShader);
	PSBlob->Release();
	FAILED_MESSAGE_RETURN(hr, L"fail to create pixel shader..");

	g_VertexCount = 8;
	g_VertexSize = sizeof(float) * 7;
	float vertices[] =
	{
		-1.0f, +1.0f, -1.0f,
		+0.0f, +0.0f, +1.0f, +1.0f,

		+1.0f, +1.0f, -1.0f,
		+0.0f, +1.0f, +0.0f, +1.0f,

		+1.0f, +1.0f, +1.0f,
		+0.0f, +1.0f, +1.0f, +1.0f,

		-1.0f, +1.0f, +1.0f,
		+1.0f, +0.0f, +0.0f, +1.0f,

		-1.0f, -1.0f, -1.0f,
		+1.0f, +0.0f, +1.0f, +1.0f,

		+1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, +0.0f, +1.0f,

		+1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f, +1.0f,

		-1.0f, -1.0f, +1.0f,
		+0.0f, +0.0f, +0.0f, +1.0f,
	};
	hr = CreateVertexBufferInline(g_D3D11Device, &g_D3D11VertexBuffer, g_VertexSize, g_VertexCount, vertices);
	FAILED_MESSAGE_RETURN(hr, L"fail to create vertex buffer..");

	g_PrimCount = 12;
	g_PrimSize = sizeof(CHAR) * 3;
	CHAR indices[] = {
		3, 1, 0,
		2, 1, 3,
		0, 5, 4,
		1, 5, 0,
		3, 4, 7,
		0, 4, 3,
		1, 6, 5,
		2, 6, 1,
		2, 7, 6,
		3, 7, 2,
		6, 4, 5,
		7, 4, 6,
	};

	hr = CreateIndexBufferInline(g_D3D11Device, &g_D3D11IndexBuffer, g_PrimSize, g_PrimCount, indices);
	FAILED_MESSAGE_RETURN(hr, L"fail to create index buffer..");

	hr = CreateConstantBufferInline(g_D3D11Device, &g_D3D11ConstantBuffer, sizeof(Tutorial4ConstantBuffer));
	FAILED_MESSAGE_RETURN(hr, L"fail to create constant buffer..");

	g_World = DirectX::XMMatrixIdentity();
	XMVECTOR 
		eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f),
		at  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		up  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = DirectX::XMMatrixLookAtLH(eye, at, up);
	g_Projection = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV2, g_D3D11ViewPort.Width / g_D3D11ViewPort.Height, 0.01f, 100.0f);

	Tutorial4ConstantBuffer constantBuffer;
	constantBuffer.world = (g_World);
	constantBuffer.view = (g_View);
	constantBuffer.projection = (g_Projection);
	g_D3D11ImmediateContext->UpdateSubresource(g_D3D11ConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

	return hr;
}

HRESULT DXEntryInit(HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, UINT maxFrameRate, bool debug)
{
	g_hWnd = hWnd;
	g_hInstance = hInstance;

	HRESULT hr = S_OK;

	hr = DXDeviceInit(width, height, maxFrameRate, debug);
	FAILED_MESSAGE_RETURN(hr, L"fail to initialize device..");

	hr = DXShaderResourceInit(L"./Tutorial5.hlsl", debug);
	FAILED_MESSAGE_RETURN(hr, L"fail to create resource view..");

	return hr;
}

void DXEntryClean()
{
	if (g_D3D11ImmediateContext) g_D3D11ImmediateContext->ClearState();

	if (g_D3D11VertexBuffer) g_D3D11VertexBuffer->Release();
	if (g_D3D11VertexLayout) g_D3D11VertexLayout->Release();
	if (g_D3D11VertexShader) g_D3D11VertexShader->Release();
	if (g_D3D11PixelShader) g_D3D11PixelShader->Release();

	if (g_D3D11RenderTargetView) g_D3D11RenderTargetView->Release();
	if (g_DXGISwapChain) g_DXGISwapChain->Release();
	if (g_D3D11ImmediateContext) g_D3D11ImmediateContext->Release();
	if (g_D3D11Device) g_D3D11Device->Release();

	if (g_D3D11InputElementDescArray) free(g_D3D11InputElementDescArray);
}

void UpdateWorldMatrices()
{
	static float time = 0.f;

	static DWORD startTime = GetTickCount();
	DWORD currentTime = GetTickCount();
	time = (currentTime - startTime) / 1000.0f;

	g_World = DirectX::XMMatrixRotationY(time);

	XMMATRIX spin = XMMatrixRotationZ(-time);
	XMMATRIX orbit = XMMatrixRotationY(-time * 2.0f);
	XMMATRIX translate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	g_World2 = scale * spin * translate * orbit;
}

void DrawCube(DirectX::XMMATRIX worldMatrix)
{
	Tutorial4ConstantBuffer constantBuffer;
	constantBuffer.world = worldMatrix;
	constantBuffer.view = g_View;
	constantBuffer.projection = g_Projection;
	g_D3D11ImmediateContext->UpdateSubresource(g_D3D11ConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

	g_D3D11ImmediateContext->IASetInputLayout(g_D3D11VertexLayout);
	UINT offset = 0;
	g_D3D11ImmediateContext->IASetVertexBuffers(0, 1, &g_D3D11VertexBuffer, &g_VertexSize, &offset);
	g_D3D11ImmediateContext->IASetIndexBuffer(g_D3D11IndexBuffer, DXGI_FORMAT_R8_UINT, 0);
	g_D3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_D3D11ImmediateContext->VSSetShader(g_D3D11VertexShader, nullptr, 0);
	g_D3D11ImmediateContext->VSSetConstantBuffers(0, 1, &g_D3D11ConstantBuffer);
	g_D3D11ImmediateContext->PSSetShader(g_D3D11PixelShader, nullptr, 0);
	g_D3D11ImmediateContext->DrawIndexed(g_PrimCount * 3, 0, 0);
}

void DXEntryFrameUpdate()
{
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_STENCIL, 0.0f, 0);
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_D3D11ImmediateContext->ClearRenderTargetView(g_D3D11RenderTargetView, clearColor);

	g_D3D11ImmediateContext->RSSetViewports(1, &g_D3D11ViewPort);
	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);

	UpdateWorldMatrices();
	DrawCube(g_World);
	DrawCube(g_World2);

	g_DXGISwapChain->Present(0, 0);
}
