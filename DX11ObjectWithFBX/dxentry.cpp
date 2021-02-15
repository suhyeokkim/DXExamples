#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <vector>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <mutex>
#include <pix3.h>
#include <dinput.h>
#include <functional>

using namespace DirectX;

#include "defined.h"
#include "dxentry.h"
#include "dxutil.h"
#include "dxres.h"

// windonw instances
HWND g_hWnd;
HINSTANCE g_hInstance;

// DXGI objects
IDXGIFactory* g_DXGIFactory;
IDXGISwapChain* g_DXGISwapChain;

/* D3D11 device objects */
ID3D11Device* g_D3D11Device;
ID3D11DeviceContext* g_D3D11ImmediateContext;
bool g_IsHDR;
UINT g_MaxFrameRate;
ID3D11RenderTargetView* g_D3D11RenderTargetView;
ID3D11Texture2D* g_D3D11DepthStencilTexture;
ID3D11DepthStencilView* g_D3D11DepthStencialView;
std::mutex g_ContextMutex;

IDirectInput8* g_Input;
IDirectInputDevice8* g_KeyInputDevice;
unsigned char g_KeyState[256];

/* D3D11 shader resources */
D3D11_VIEWPORT g_D3D11ViewPort;
XMMATRIX g_World;
XMMATRIX g_View;
XMMATRIX g_Projection;

Allocaters g_GlobalAllocaters{ malloc, realloc, free };
DX11Resources g_ExternalResources;
uint g_DependancyCount;
DX11PipelineDependancy* g_Dependancies;
DX11ContextState g_ContextState;

struct ImmutableCB
{
	Vector4f position;
};

struct OnResizeCB
{
	DirectX::XMMATRIX projectionMatrix;
};

struct OnFrameCB
{
	DirectX::XMMATRIX transfromMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX worldMatrix;
	Vector4f view;
	Vector4f baseColor;
	Vector4f tintColor;
	Vector4f lightDir;
};

HRESULT DXDeviceInit(UINT width, UINT height, UINT maxFrameRate, bool debug)
{
	HRESULT hr = S_OK;

	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&g_DXGIFactory));
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create DXGIFactory2..");

	D3D_FEATURE_LEVEL maxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL featureLevels[] = {
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
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create D3D11Device..");

	hr = CreateSwapChainInline(g_hWnd, g_DXGIFactory, g_D3D11Device, &g_DXGISwapChain, width, height, g_MaxFrameRate = maxFrameRate, g_IsHDR = false);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create SwapChain..");

	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to get buffer from swapchain..");

	hr = g_D3D11Device->CreateRenderTargetView(backBuffer, nullptr, &g_D3D11RenderTargetView);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create rendertargetview..");

	hr = CreateDepthStencilInline(g_D3D11Device, &g_D3D11DepthStencilTexture, &g_D3D11DepthStencialView, width, height);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create depth-stencil buffer and view..");

	g_D3D11ViewPort.Width = (FLOAT)width;
	g_D3D11ViewPort.Height = (FLOAT)height;
	g_D3D11ViewPort.MinDepth = 0.0f;
	g_D3D11ViewPort.MaxDepth = 1.0f;
	g_D3D11ViewPort.TopLeftX = 0;
	g_D3D11ViewPort.TopLeftY = 0;
	
	hr = DirectInput8Create(g_hInstance, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (void**)&g_Input, nullptr);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create direct input..");
	hr = g_Input->CreateDevice(GUID_SysKeyboard, &g_KeyInputDevice, nullptr);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create direct key input device..");
	hr = g_KeyInputDevice->SetDataFormat(&c_dfDIKeyboard);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to set data format as keyboard..");
	hr = g_KeyInputDevice->Acquire();
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to aquired from keyboard..");

	return hr;
}

// TODO:: naive resizing function..
HRESULT DXEntryResize(UINT width, UINT height)
{
	if (!g_D3D11ImmediateContext) return S_OK;

	std::lock_guard<std::mutex> lock(g_ContextMutex);

	HRESULT hr = S_OK;

	g_D3D11ImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	g_D3D11RenderTargetView->Release();
	g_DXGISwapChain->Release();

	hr = CreateSwapChainInline(g_hWnd, g_DXGIFactory, g_D3D11Device, &g_DXGISwapChain, width, height, g_MaxFrameRate, g_IsHDR);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create SwapChain..");

	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to get buffer from swapchain..");

	hr = g_D3D11Device->CreateRenderTargetView(backBuffer, nullptr, &g_D3D11RenderTargetView);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create resized RTV..");
	backBuffer->Release();

	g_D3D11DepthStencilTexture->Release();
	g_D3D11DepthStencialView->Release();

	hr = CreateDepthStencilInline(g_D3D11Device, &g_D3D11DepthStencilTexture, &g_D3D11DepthStencialView, width, height);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create depth-stencil buffer and view..");

	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);

	g_D3D11ViewPort.Width = (FLOAT)width;
	g_D3D11ViewPort.Height = (FLOAT)height;
	g_D3D11ViewPort.MinDepth = 0.0f;
	g_D3D11ViewPort.MaxDepth = 1.0f;
	g_D3D11ViewPort.TopLeftX = 0;
	g_D3D11ViewPort.TopLeftY = 0;

	OnResizeCB resizeCB{ 
		g_Projection = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, g_D3D11ViewPort.Width / g_D3D11ViewPort.Height, 0.01f, 100.0f)
	};
	UploadConstantBuffer(&g_ExternalResources, g_D3D11ImmediateContext, 1, &resizeCB);

	return hr;
}

HRESULT PipelineDependancySet();

HRESULT DXShaderResourceInit(bool debug)
{
	// load dx11 object 
	// external resource depandency
	FBXChunk c;
	FBXLoadOptionChunk opt;
	opt.flipV = 1;
	FALSE_ERROR_MESSAGE_RETURN_CODE(
		ImportFBX(L"./char_max.fbx", c, &opt), L"fail to load fbxfile as \"ImportFBX\"", E_FAIL
	);

	const wchar_t* textureName[] = { L"./char_max.png", L"./face_00.png" };
	ShaderCompileDesc shaderDescArray[] = { { L"./object.hlsl", "vertex", "vs_5_0" }, { L"./object.hlsl", "pixel", "ps_5_0" } };
	DX11InputLayoutDesc inputLayoutDescArray[] = { { 0, 0 } };
	D3D11_SAMPLER_DESC samplerDescs[] = { 
		{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, 
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			0,
			0,
			D3D11_COMPARISON_NEVER,
			{ 0, 0, 0, 0 },
			0,
			0
		} 
	};
	size_t cbSize[] = { sizeof(ImmutableCB), sizeof(OnResizeCB), sizeof(OnFrameCB) };
	FAILED_ERROR_MESSAGE_RETURN(
		LoadGeometryFromFBXChunk(
			&g_ExternalResources,  &g_GlobalAllocaters, g_D3D11Device, 
			1, &c
		),
		L"fail to load geometry.."
	);
	FAILED_ERROR_MESSAGE_RETURN(
		LoadTexture2DAndSRVFromDirectories(
			&g_ExternalResources, &g_GlobalAllocaters, g_D3D11Device, 
			2, textureName
		),
		L"fail to load textures.."
	);
	FAILED_ERROR_MESSAGE_RETURN(
		LoadShaderFromDirectoriesAndInputLayout(
			&g_ExternalResources, &g_GlobalAllocaters, g_D3D11Device, 
			ARRAYSIZE(shaderDescArray), shaderDescArray, ARRAYSIZE(inputLayoutDescArray), inputLayoutDescArray
		),
		L"fail to compile shaderes.."
	);
	FAILED_ERROR_MESSAGE_RETURN(
		CreateSamplerStates(
			&g_ExternalResources, &g_GlobalAllocaters, g_D3D11Device,
			ARRAYSIZE(samplerDescs), samplerDescs
		),
		L"fail to create sampler states.."
	);
	FAILED_ERROR_MESSAGE_RETURN(
		CreateConstantBuffers(
			&g_ExternalResources, &g_GlobalAllocaters, g_D3D11Device,
			ARRAYSIZE(cbSize), cbSize
		),
		L"fail to create constant buffers.."
	);

	// pipeline dependancy from resources
	PipelineDependancySet();

	// context prepare
	DrawingContextStatePrepare(&g_ContextState, &g_GlobalAllocaters, g_DependancyCount, g_Dependancies);

	// upload constant buffer
	ImmutableCB immutableCB { Vector4f() };
	UploadConstantBuffer(&g_ExternalResources, g_D3D11ImmediateContext, 0, &immutableCB);
	OnResizeCB resizeCB { 
		g_Projection = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, g_D3D11ViewPort.Width / g_D3D11ViewPort.Height, 0.01f, 100000.0f)
	};
	UploadConstantBuffer(&g_ExternalResources, g_D3D11ImmediateContext, 1, &resizeCB);

	return S_OK;
}

HRESULT DXEntryInit(HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, UINT maxFrameRate, bool debug)
{
	DWORD startMilliSecond = GetTickCount();
	HRESULT hr = S_OK;
	{
		g_hWnd = hWnd;
		g_hInstance = hInstance;

		hr = DXDeviceInit(width, height, maxFrameRate, debug);
		FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to initialize device..");
		hr = DXShaderResourceInit(debug);
		FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create resource view..");
	}
	printf("intialize time : %.3f\n", (float)(GetTickCount()- startMilliSecond) / 1000.f);

	return hr;
}

void DXEntryClean()
{
	if (g_D3D11ImmediateContext) g_D3D11ImmediateContext->ClearState();

	if (g_D3D11RenderTargetView) g_D3D11RenderTargetView->Release();
	if (g_DXGISwapChain) g_DXGISwapChain->Release();
	if (g_D3D11ImmediateContext) g_D3D11ImmediateContext->Release();
	if (g_D3D11Device) g_D3D11Device->Release();

	if (g_KeyInputDevice) 
	{
		g_KeyInputDevice->Unacquire();
		g_KeyInputDevice->Release();
		g_KeyInputDevice = nullptr;
	}

	if (g_Input)
	{
		g_Input->Release();
		g_Input = nullptr;
	}

	ReleaseResources(&g_ExternalResources, &g_GlobalAllocaters);
	ReleaseContext(&g_ContextState, &g_GlobalAllocaters);

	if (g_Dependancies)
		for (int i = 0; i < g_DependancyCount; i++)
			ReleaseDependancy(g_Dependancies + i, &g_GlobalAllocaters);
}

Vector4f 
	g_CurrentPos = Vector4f(0.0f, 0.f, 100.0f, 0.0f),
	g_ObjectPos = Vector4f(0.0f, 0.f, 0.0f, 0.0f);

void ReadKey();
void UpdateConstantBuffer();
void Render();
void RenderDependancy();

int g_FrameCount = 0;
double g_AvgFrameTime = 0.f;

void DXEntryFrameUpdate()
{
	g_FrameCount++;
	auto start = std::chrono::high_resolution_clock::now();
	{
		ReadKey();
		UpdateConstantBuffer();
		/*/
		Render();
		/*/
		RenderDependancy();
		//*/
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	double tempTime = microseconds / 1000.0;
	g_AvgFrameTime = g_AvgFrameTime * (g_FrameCount - 1) / g_FrameCount + tempTime / g_FrameCount;
	printf("temp frame time : %.4lfms, avg frame time : %.4lfms\n", tempTime, g_AvgFrameTime);
}

void UpdateConstantBuffer()
{
	DirectX::FXMVECTOR
		scale = XMVectorSet(0.2, 0.2, 0.2, 0.2),
		rotationOrigin = XMVectorSet(0.0, 0.0, 0.0, 0.0),
		rotationQuat = XMVectorSet(0, 0, 0, 1),
		translate = XMVectorSet(0, 0, 0, 1);
	g_World = DirectX::XMMatrixAffineTransformation(
		scale, rotationOrigin, rotationQuat, translate
	);

	static float time = 0.f;
	static DWORD startTime = GetTickCount();
	DWORD currentTime = GetTickCount();
	time = (currentTime - startTime) / 1000.0f;

	XMVECTOR
		eye = XMLoadFloat4((XMFLOAT4*)&g_CurrentPos),
		at = XMLoadFloat4((XMFLOAT4*)&g_ObjectPos),
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);
	OnFrameCB cb{
		g_World * viewMatrix * g_Projection,
		viewMatrix,
		g_World,
		(g_ObjectPos - g_CurrentPos).normalized(),
		Vector4f(1, 0, 0, 0),
		Vector4f::One(),
		Vector4f(1, 1, 1, 0).normalized()
	};
	UploadConstantBuffer(&g_ExternalResources, g_D3D11ImmediateContext, 2, &cb);
}

void ReadKey()
{
	HRESULT hr = g_KeyInputDevice->GetDeviceState(sizeof(g_KeyState), &g_KeyState);

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			g_KeyInputDevice->Acquire();
	}

	const float step = 0.1f;

	if (g_KeyState[DIK_W] & 0x80)
		g_CurrentPos.z += step;
	else if (g_KeyState[DIK_S] & 0x80)
		g_CurrentPos.z -= step;

	if (g_KeyState[DIK_A] & 0x80)
		g_CurrentPos.x += step;
	else if (g_KeyState[DIK_D] & 0x80)
		g_CurrentPos.x -= step;

	if (g_KeyState[DIK_Q] & 0x80)
		g_CurrentPos.y += step;
	else if (g_KeyState[DIK_E] & 0x80)
		g_CurrentPos.y -= step;
}

void RenderDependancy()
{
	std::lock_guard<std::mutex> lock(g_ContextMutex);

	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_STENCIL, 0.0f, 0);
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_D3D11ImmediateContext->ClearRenderTargetView(g_D3D11RenderTargetView, clearColor);

	g_D3D11ImmediateContext->RSSetViewports(1, &g_D3D11ViewPort);
	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);
	
	DrawExplicitly(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DependancyCount, g_Dependancies);

	g_DXGISwapChain->Present(0, 0);
}

void Render()
{
	std::lock_guard<std::mutex> lock(g_ContextMutex);

	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_STENCIL, 0.0f, 0);
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_D3D11ImmediateContext->ClearRenderTargetView(g_D3D11RenderTargetView, clearColor);

	g_D3D11ImmediateContext->RSSetViewports(1, &g_D3D11ViewPort);
	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);

	auto maxGeometry = g_ExternalResources.geometryChunks[0];
	auto geometryLayout = g_ExternalResources.vertexLayouts[maxGeometry.vertexLayoutIndex];
	auto objectShaderFile = g_ExternalResources.shaderFiles[0];
	auto vertexShader = objectShaderFile.vsBlobs[0].vs;
	auto pixelShader = objectShaderFile.psBlobs[0].ps;
	auto inputLayout = g_ExternalResources.inputLayouts[0].inputLayout;
	auto sampler = g_ExternalResources.samplerStates[0];
	auto torsoSRV = g_ExternalResources.srvs[g_ExternalResources.texture2Ds[0].srvIndex];
	auto immutableCB = g_ExternalResources.constantBuffers[0];
	auto resizeCB = g_ExternalResources.constantBuffers[1];
	auto frameCB = g_ExternalResources.constantBuffers[2];

	g_D3D11ImmediateContext->IASetInputLayout(inputLayout);
	uint offset = 0;
	g_D3D11ImmediateContext->IASetVertexBuffers(0, 1, &maxGeometry.vertexBuffer, &geometryLayout.vertexSize, &offset);
	g_D3D11ImmediateContext->IASetIndexBuffer(maxGeometry.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_D3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_D3D11ImmediateContext->VSSetShader(vertexShader, nullptr, 0);
	g_D3D11ImmediateContext->VSSetConstantBuffers(0, 1, &immutableCB);
	g_D3D11ImmediateContext->VSSetConstantBuffers(1, 1, &resizeCB);
	g_D3D11ImmediateContext->VSSetConstantBuffers(2, 1, &frameCB);
	g_D3D11ImmediateContext->VSSetSamplers(0, 1, &sampler);
	g_D3D11ImmediateContext->PSSetShader(pixelShader, nullptr, 0);
	g_D3D11ImmediateContext->PSSetConstantBuffers(2, 1, &frameCB);
	g_D3D11ImmediateContext->PSSetShaderResources(0, 1, &torsoSRV);
	g_D3D11ImmediateContext->PSSetSamplers(0, 1, &sampler);
	g_D3D11ImmediateContext->DrawIndexed(maxGeometry.indexCount, 0, 0);

	auto faceSRV = g_ExternalResources.srvs[g_ExternalResources.texture2Ds[1].srvIndex];
	auto faceGeometry = g_ExternalResources.geometryChunks[1];
	offset = 0;
	g_D3D11ImmediateContext->IASetVertexBuffers(0, 1, &faceGeometry.vertexBuffer, &geometryLayout.vertexSize, &offset);
	g_D3D11ImmediateContext->IASetIndexBuffer(faceGeometry.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_D3D11ImmediateContext->PSSetShaderResources(0, 1, &faceSRV);
	g_D3D11ImmediateContext->DrawIndexed(faceGeometry.indexCount, 0, 0);

	g_DXGISwapChain->Present(0, 0);
}

HRESULT PipelineDependancySet()
{
	g_DependancyCount = 2;
	g_Dependancies = (DX11PipelineDependancy*)g_GlobalAllocaters.alloc(sizeof(DX11PipelineDependancy) * g_DependancyCount);

	new (g_Dependancies + 0) DX11PipelineDependancy();
	{
		g_Dependancies[0].input.inputLayoutIndex = 0;
		g_Dependancies[0].input.geometryIndex = 0;
		g_Dependancies[0].input.vertexBufferOffset = 0;
		g_Dependancies[0].input.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		g_Dependancies[0].vs.shaderFileIndex = 0;
		g_Dependancies[0].vs.shaderIndex = 0;

		g_Dependancies[0].vs.constantBufferCount = 3;
		g_Dependancies[0].vs.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * g_Dependancies[0].vs.constantBufferCount
			);
		g_Dependancies[0].vs.constantBuffers[0].slotOrRegister = 0;
		g_Dependancies[0].vs.constantBuffers[0].indexCount = 1;
		g_Dependancies[0].vs.constantBuffers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].vs.constantBuffers[0].indices[0] = 0;
		g_Dependancies[0].vs.constantBuffers[1].slotOrRegister = 1;
		g_Dependancies[0].vs.constantBuffers[1].indexCount = 1;
		g_Dependancies[0].vs.constantBuffers[1].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].vs.constantBuffers[1].indices[0] = 1;
		g_Dependancies[0].vs.constantBuffers[2].slotOrRegister = 2;
		g_Dependancies[0].vs.constantBuffers[2].indexCount = 1;
		g_Dependancies[0].vs.constantBuffers[2].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].vs.constantBuffers[2].indices[0] = 2;

		g_Dependancies[0].vs.samplerCount = 1;
		g_Dependancies[0].vs.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * g_Dependancies[0].vs.samplerCount
			);
		g_Dependancies[0].vs.samplers[0].slotOrRegister = 0;
		g_Dependancies[0].vs.samplers[0].indexCount = 1;
		g_Dependancies[0].vs.samplers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].vs.samplers[0].indices[0] = 0;

		g_Dependancies[0].ps.shaderFileIndex = 0;
		g_Dependancies[0].ps.shaderIndex = 0;

		g_Dependancies[0].ps.constantBufferCount = 1;
		g_Dependancies[0].ps.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * g_Dependancies[0].ps.constantBufferCount
			);
		g_Dependancies[0].ps.constantBuffers[0].slotOrRegister = 2;
		g_Dependancies[0].ps.constantBuffers[0].indexCount = 1;
		g_Dependancies[0].ps.constantBuffers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].ps.constantBuffers[0].indices[0] = 2;

		g_Dependancies[0].ps.srvCount = 1;
		g_Dependancies[0].ps.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * g_Dependancies[0].ps.srvCount
			);
		g_Dependancies[0].ps.srvs[0].slotOrRegister = 0;
		g_Dependancies[0].ps.srvs[0].indexCount = 1;
		g_Dependancies[0].ps.srvs[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].ps.srvs[0].indices[0] = 0;

		g_Dependancies[0].ps.samplerCount = 1;
		g_Dependancies[0].ps.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * g_Dependancies[0].ps.samplerCount
			);
		g_Dependancies[0].ps.samplers[0].slotOrRegister = 0;
		g_Dependancies[0].ps.samplers[0].indexCount = 1;
		g_Dependancies[0].ps.samplers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[0].ps.samplers[0].indices[0] = 0;

		g_Dependancies[0].drawType = DX11PipelineDependancy::DrawType::DrawIndexed;
		g_Dependancies[0].argsAsDraw.drawIndexedArgs.indexCount =
			g_ExternalResources.geometryChunks[g_Dependancies[0].input.geometryIndex].indexCount;
		g_Dependancies[0].argsAsDraw.drawIndexedArgs.startIndexLocation = 0;
		g_Dependancies[0].argsAsDraw.drawIndexedArgs.baseVertexLocation = 0;
	}
	new (g_Dependancies + 1) DX11PipelineDependancy();
	{
		g_Dependancies[1].input.inputLayoutIndex = 0;
		g_Dependancies[1].input.geometryIndex = 1;
		g_Dependancies[1].input.vertexBufferOffset = 0;
		g_Dependancies[1].input.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		g_Dependancies[1].vs.shaderFileIndex = 0;
		g_Dependancies[1].vs.shaderIndex = 0;

		g_Dependancies[1].vs.constantBufferCount = 3;
		g_Dependancies[1].vs.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * g_Dependancies[1].vs.constantBufferCount
			);
		g_Dependancies[1].vs.constantBuffers[0].slotOrRegister = 0;
		g_Dependancies[1].vs.constantBuffers[0].indexCount = 1;
		g_Dependancies[1].vs.constantBuffers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].vs.constantBuffers[0].indices[0] = 0;
		g_Dependancies[1].vs.constantBuffers[1].slotOrRegister = 1;
		g_Dependancies[1].vs.constantBuffers[1].indexCount = 1;
		g_Dependancies[1].vs.constantBuffers[1].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].vs.constantBuffers[1].indices[0] = 1;
		g_Dependancies[1].vs.constantBuffers[2].slotOrRegister = 2;
		g_Dependancies[1].vs.constantBuffers[2].indexCount = 1;
		g_Dependancies[1].vs.constantBuffers[2].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].vs.constantBuffers[2].indices[0] = 2;

		g_Dependancies[1].vs.samplerCount = 1;
		g_Dependancies[1].vs.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * g_Dependancies[1].vs.samplerCount
			);
		g_Dependancies[1].vs.samplers[0].slotOrRegister = 0;
		g_Dependancies[1].vs.samplers[0].indexCount = 1;
		g_Dependancies[1].vs.samplers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].vs.samplers[0].indices[0] = 0;

		g_Dependancies[1].ps.shaderFileIndex = 0;
		g_Dependancies[1].ps.shaderIndex = 0;

		g_Dependancies[1].ps.constantBufferCount = 1;
		g_Dependancies[1].ps.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * g_Dependancies[1].ps.constantBufferCount
			);
		g_Dependancies[1].ps.constantBuffers[0].slotOrRegister = 2;
		g_Dependancies[1].ps.constantBuffers[0].indexCount = 1;
		g_Dependancies[1].ps.constantBuffers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].ps.constantBuffers[0].indices[0] = 2;

		g_Dependancies[1].ps.srvCount = 1;
		g_Dependancies[1].ps.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * g_Dependancies[1].ps.srvCount
			);
		g_Dependancies[1].ps.srvs[0].slotOrRegister = 0;
		g_Dependancies[1].ps.srvs[0].indexCount = 1;
		g_Dependancies[1].ps.srvs[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].ps.srvs[0].indices[0] = 1;

		g_Dependancies[1].ps.samplerCount = 1;
		g_Dependancies[1].ps.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)g_GlobalAllocaters.alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * g_Dependancies[1].ps.samplerCount
			);
		g_Dependancies[1].ps.samplers[0].slotOrRegister = 0;
		g_Dependancies[1].ps.samplers[0].indexCount = 1;
		g_Dependancies[1].ps.samplers[0].indices = (uint*)g_GlobalAllocaters.alloc(sizeof(uint));
		g_Dependancies[1].ps.samplers[0].indices[0] = 0;

		g_Dependancies[1].drawType = DX11PipelineDependancy::DrawType::DrawIndexed;
		g_Dependancies[1].argsAsDraw.drawIndexedArgs.indexCount =
			g_ExternalResources.geometryChunks[g_Dependancies[1].input.geometryIndex].indexCount;
		g_Dependancies[1].argsAsDraw.drawIndexedArgs.startIndexLocation = 0;
		g_Dependancies[1].argsAsDraw.drawIndexedArgs.baseVertexLocation = 0;
	}

	return S_OK;
}