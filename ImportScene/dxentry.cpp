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

_declspec(align(16)) struct ImmutableCB
{
	Vector4f position;
};

_declspec(align(16)) struct OnResizeCB
{
	DirectX::XMMATRIX projectionMatrix;
};

_declspec(align(16)) struct OnFrameCB
{
	DirectX::XMMATRIX transfromMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX worldMatrix;
	Vector4f view;
	Vector4f baseColor;
	Vector4f tintColor;
	Vector4f lightDir;
};

_declspec(align(16)) struct SkinningConfigCB
{
	uint vertexCount;
	uint boneCount;
	uint frameIndex;
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

HRESULT PipelineDependancySet(uint* dependCount, DX11PipelineDependancy** depends, const Allocaters* allocs);

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
	FBXChunkConfig::FBXMeshConfig meshConfigs[] = { true, true };
	FBXChunkConfig config { meshConfigs };

	const wchar_t* textureName[] = { L"./char_max.png", L"./face_00.png" };
	DX11ShaderCompileDesc shaderDescArray[] = { { L"./object.hlsl", "vertex", "vs_5_0" }, { L"./object.hlsl", "pixel", "ps_5_0" }, { L"./lbs_compute.hlsl", "lbs", "cs_5_0" } };
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
	uint cbSize[] = { sizeof(ImmutableCB), sizeof(OnResizeCB), sizeof(OnFrameCB), sizeof(SkinningConfigCB) };

	DX11ResourceDesc resDesc = { 
		1, &c, &config,
		ARRAYSIZE(shaderDescArray), shaderDescArray, 
		ARRAYSIZE(inputLayoutDescArray), inputLayoutDescArray, 
		ARRAYSIZE(textureName), textureName, 0, nullptr,
		ARRAYSIZE(cbSize), cbSize,
		ARRAYSIZE(samplerDescs), samplerDescs
	};

	DX11RawResourceBuffer buffer;
	FAILED_ERROR_MESSAGE_RETURN(
		LoadDX11Resoureces(
			&g_ExternalResources, &buffer, &resDesc, &g_GlobalAllocaters, g_D3D11Device
		),
		L"fail to load dx11 resources.."
	);

	// pipeline dependancy from resources
	PipelineDependancySet(&g_DependancyCount, &g_Dependancies, &g_GlobalAllocaters);

	// context prepare
	DependancyContextStatePrepare(&g_ContextState, &g_GlobalAllocaters, g_DependancyCount, g_Dependancies);

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
		for (uint i = 0; i < g_DependancyCount; i++)
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
DWORD g_StartTick = GetTickCount();
double g_AvgFrameTime = 0.f;

void DXEntryFrameUpdate()
{
	g_FrameCount++;
	auto start = std::chrono::high_resolution_clock::now();
	{
		ReadKey();
		UpdateConstantBuffer();
		//*/
		Render();
		/*/
		RenderDependancy();
		//*/
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	double tempTime = microseconds / 1000.0;
	g_AvgFrameTime = g_AvgFrameTime * (g_FrameCount - 1) / g_FrameCount + tempTime / g_FrameCount;
}

void UpdateConstantBuffer()
{
	DirectX::FXMVECTOR
		scale = XMVectorSet(0.2f, 0.2f, 0.2f, 0.2f),
		rotationOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
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
	
	ExecuteExplicitly(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DependancyCount, g_Dependancies);

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

	auto immutableCB = g_ExternalResources.buffers[g_ExternalResources.constantBufferIndices[0]];
	auto resizeCB = g_ExternalResources.buffers[g_ExternalResources.constantBufferIndices[1]];
	auto frameCB = g_ExternalResources.buffers[g_ExternalResources.constantBufferIndices[2]];
	auto skinningConfigCB = g_ExternalResources.buffers[g_ExternalResources.constantBufferIndices[3]];
	auto sampler = g_ExternalResources.samplerStates[0];

	auto objectShaderFile = g_ExternalResources.shaderFiles[0];
	auto vertexShader = g_ExternalResources.shaders.vss[objectShaderFile.vsIndices[0]];
	auto pixelShader = g_ExternalResources.shaders.pss[objectShaderFile.psIndices[0]];

	auto anim = g_ExternalResources.anims[0];
	auto boneBindPoseSRV = g_ExternalResources.srvs[g_ExternalResources.binePoseTransformSRVIndex];
	auto bonePoseSRV = g_ExternalResources.srvs[anim.animPoseTransformSRVIndex];

	for (int i = 0; i < g_ExternalResources.geometryCount; i++)
	{
		auto geometry = g_ExternalResources.geometryChunks[i];
		auto lbsCompute = g_ExternalResources.shaders.css[0];
		auto skinningVertexData = g_ExternalResources.srvs[geometry.vertexDataSRVIndex];
		auto skinningUAV = g_ExternalResources.uavs[i];
		UINT offsets[2] = { 0, 0 };

		SkinningConfigCB cb;
		cb.boneCount = g_ExternalResources.boneCount;
		cb.frameIndex = ((GetTickCount() - g_StartTick) / 33) % anim.frameKeyCount;
		cb.vertexCount = geometry.vertexCount;

		g_D3D11ImmediateContext->UpdateSubresource(skinningConfigCB, 0, nullptr, &cb, 0, 0);

		g_D3D11ImmediateContext->CSSetShader(lbsCompute, nullptr, 0);
		g_D3D11ImmediateContext->CSSetConstantBuffers(0, 1, &skinningConfigCB);
		g_D3D11ImmediateContext->CSSetShaderResources(0, 1, &skinningVertexData);
		g_D3D11ImmediateContext->CSSetShaderResources(1, 1, &boneBindPoseSRV);
		g_D3D11ImmediateContext->CSSetShaderResources(2, 1, &bonePoseSRV);
		g_D3D11ImmediateContext->CSSetUnorderedAccessViews(0, 1, &skinningUAV, offsets);

		g_D3D11ImmediateContext->Dispatch(geometry.vertexCount, 1, 1);

		g_D3D11ImmediateContext->CopyResource(
			g_ExternalResources.buffers[geometry.vertexBufferIndex],
			g_ExternalResources.buffers[geometry.vertexStreamBufferIndex]
			);

		auto geometryLayout = g_ExternalResources.vertexLayouts[geometry.vertexLayoutIndex];
		auto inputLayout = g_ExternalResources.inputLayoutItems[g_ExternalResources.inputLayouts[0].inputLayoutIndex];
		auto texSRV = g_ExternalResources.srvs[g_ExternalResources.texture2Ds[i].srvIndex];

		g_D3D11ImmediateContext->IASetInputLayout(inputLayout);
		uint offset = 0;
		g_D3D11ImmediateContext->IASetVertexBuffers(0, 1, &g_ExternalResources.buffers[geometry.vertexBufferIndex], &geometryLayout.vertexSize, &offset);
		g_D3D11ImmediateContext->IASetIndexBuffer(g_ExternalResources.buffers[geometry.indexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
		g_D3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_D3D11ImmediateContext->VSSetShader(vertexShader, nullptr, 0);
		g_D3D11ImmediateContext->VSSetConstantBuffers(0, 1, &immutableCB);
		g_D3D11ImmediateContext->VSSetConstantBuffers(1, 1, &resizeCB);
		g_D3D11ImmediateContext->VSSetConstantBuffers(2, 1, &frameCB);

		g_D3D11ImmediateContext->PSSetShader(pixelShader, nullptr, 0);
		g_D3D11ImmediateContext->PSSetConstantBuffers(2, 1, &frameCB);
		g_D3D11ImmediateContext->PSSetShaderResources(0, 1, &texSRV);
		g_D3D11ImmediateContext->PSSetSamplers(0, 1, &sampler);

		g_D3D11ImmediateContext->DrawIndexed(geometry.indexCount, 0, 0);
	}

	g_DXGISwapChain->Present(0, 0);
}

HRESULT PipelineDependancySet(uint* dependCount, DX11PipelineDependancy** depends, const Allocaters* allocs)
{
	*dependCount = 6;
	*depends = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * g_DependancyCount);
	memset(*depends, 0, sizeof(DX11PipelineDependancy) * g_DependancyCount);

	{
		(*depends)[0].pipelineKind = PIPELINE_KIND::COMPUTE;
		DX11ComputePipelineDependancy& compute = ((*depends))[0].compute;
		DX11ShaderResourceDependancy& srd = compute.resources;

		srd.shaderFileIndex = 1;
		srd.shaderIndex = 0;

		srd.srvCount = 3;
		srd.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * srd.srvCount
			);
		srd.srvs[0].indexCount = 3;
		srd.srvs[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.srvs[0].indices[0] = 1;
		srd.srvs[0].slotOrRegister = 0;
		srd.srvs[1].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.srvs[1].indices[0] = 3;
		srd.srvs[1].slotOrRegister = 1;
		srd.srvs[2].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.srvs[2].indices[0] = 4;
		srd.srvs[2].slotOrRegister = 2;

		srd.uavCount = 1;
		srd.uavs =
			(DX11ShaderResourceDependancy::DX11UAVRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11UAVRef) * srd.uavCount
			);
		srd.uavs[0].indexCount = 1;
		srd.uavs[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.uavs[0].indices[0] = 1;
		srd.uavs[0].slotOrRegister = 0;
	}
	{
		// TODO:: copy buffer to buffer index
		(*depends)[1].pipelineKind = PIPELINE_KIND::COPY;
		DX11CopyDependancy& cpy = ((*depends))[1].copy;
		cpy.kind = CopyKind::COPY_RESOURCE;
		cpy.srcBufferIndex = 0;
		cpy.dstBufferIndex = 0;
	}
	{
		(*depends)[2].pipelineKind = PIPELINE_KIND::COMPUTE;
		DX11ComputePipelineDependancy& compute = ((*depends))[1].compute;
		DX11ShaderResourceDependancy& srd = compute.resources;

		srd.shaderFileIndex = 1;
		srd.shaderIndex = 0;

		srd.srvCount = 3;
		srd.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * srd.srvCount
			);
		srd.srvs[0].indexCount = 3;
		srd.srvs[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.srvs[0].indices[0] = 2;
		srd.srvs[0].slotOrRegister = 0;
		srd.srvs[1].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.srvs[1].indices[0] = 3;
		srd.srvs[1].slotOrRegister = 1;
		srd.srvs[2].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.srvs[2].indices[0] = 4;
		srd.srvs[2].slotOrRegister = 2;

		srd.uavCount = 1;
		srd.uavs =
			(DX11ShaderResourceDependancy::DX11UAVRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11UAVRef) * srd.uavCount
			);
		srd.uavs[0].indexCount = 1;
		srd.uavs[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
		srd.uavs[0].indices[0] = 1;
		srd.uavs[0].slotOrRegister = 0;
	}
	{
		// TODO:: copy buffer to buffer index
		(*depends)[3].pipelineKind = PIPELINE_KIND::COPY;
		DX11CopyDependancy& cpy = ((*depends))[3].copy;
		cpy.kind = CopyKind::COPY_RESOURCE;
		cpy.srcBufferIndex = 0;
		cpy.dstBufferIndex = 0;
	}
	{
		(*depends)[4].pipelineKind = PIPELINE_KIND::DRAW;
		DX11DrawPipelineDependancy& draw = (*depends)[2].draw;
		new (&draw) DX11DrawPipelineDependancy();

		draw.input.inputLayoutIndex = 0;
		draw.input.geometryIndex = 0;
		draw.input.vertexBufferOffset = 0;
		draw.input.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		draw.vs.shaderFileIndex = 0;
		draw.vs.shaderIndex = 0;
		draw.vs.constantBufferCount = 3;
		draw.vs.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * draw.vs.constantBufferCount
			);
		draw.vs.constantBuffers[0].slotOrRegister = 0;
		draw.vs.constantBuffers[0].indexCount = 1;
		draw.vs.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.constantBuffers[0].indices[0] = 0;
		draw.vs.constantBuffers[1].slotOrRegister = 1;
		draw.vs.constantBuffers[1].indexCount = 1;
		draw.vs.constantBuffers[1].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.constantBuffers[1].indices[0] = 1;
		draw.vs.constantBuffers[2].slotOrRegister = 2;
		draw.vs.constantBuffers[2].indexCount = 1;
		draw.vs.constantBuffers[2].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.constantBuffers[2].indices[0] = 2;

		draw.vs.samplerCount = 1;
		draw.vs.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * draw.vs.samplerCount
			);
		draw.vs.samplers[0].slotOrRegister = 0;
		draw.vs.samplers[0].indexCount = 1;
		draw.vs.samplers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.samplers[0].indices[0] = 0;

		draw.ps.shaderFileIndex = 0;
		draw.ps.shaderIndex = 0;

		draw.ps.constantBufferCount = 1;
		draw.ps.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * draw.ps.constantBufferCount
			);
		draw.ps.constantBuffers[0].slotOrRegister = 2;
		draw.ps.constantBuffers[0].indexCount = 1;
		draw.ps.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.ps.constantBuffers[0].indices[0] = 2;

		draw.ps.srvCount = 1;
		draw.ps.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * draw.ps.srvCount
			);
		draw.ps.srvs[0].slotOrRegister = 0;
		draw.ps.srvs[0].indexCount = 1;
		draw.ps.srvs[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.ps.srvs[0].indices[0] = 0;

		draw.ps.samplerCount = 1;
		draw.ps.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * draw.ps.samplerCount
			);
		draw.ps.samplers[0].slotOrRegister = 0;
		draw.ps.samplers[0].indexCount = 1;
		draw.ps.samplers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.ps.samplers[0].indices[0] = 0;

		draw.drawType = DX11DrawPipelineDependancy::DrawType::DRAW_INDEXED;
		draw.argsAsDraw.drawIndexedArgs.indexCount =
			g_ExternalResources.geometryChunks[draw.input.geometryIndex].indexCount;
		draw.argsAsDraw.drawIndexedArgs.startIndexLocation = 0;
		draw.argsAsDraw.drawIndexedArgs.baseVertexLocation = 0;
	}
	{
		(*depends)[5].pipelineKind = PIPELINE_KIND::DRAW;
		DX11DrawPipelineDependancy& draw = ((*depends))[3].draw;
		new (&draw) DX11DrawPipelineDependancy();

		draw.input.inputLayoutIndex = 0;
		draw.input.geometryIndex = 1;
		draw.input.vertexBufferOffset = 0;
		draw.input.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		draw.vs.shaderFileIndex = 0;
		draw.vs.shaderIndex = 0;

		draw.vs.constantBufferCount = 3;
		draw.vs.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * draw.vs.constantBufferCount
			);
		draw.vs.constantBuffers[0].slotOrRegister = 0;
		draw.vs.constantBuffers[0].indexCount = 1;
		draw.vs.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.constantBuffers[0].indices[0] = 0;
		draw.vs.constantBuffers[1].slotOrRegister = 1;
		draw.vs.constantBuffers[1].indexCount = 1;
		draw.vs.constantBuffers[1].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.constantBuffers[1].indices[0] = 1;
		draw.vs.constantBuffers[2].slotOrRegister = 2;
		draw.vs.constantBuffers[2].indexCount = 1;
		draw.vs.constantBuffers[2].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.constantBuffers[2].indices[0] = 2;

		draw.vs.samplerCount = 1;
		draw.vs.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * draw.vs.samplerCount
			);
		draw.vs.samplers[0].slotOrRegister = 0;
		draw.vs.samplers[0].indexCount = 1;
		draw.vs.samplers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.vs.samplers[0].indices[0] = 0;

		draw.ps.shaderFileIndex = 0;
		draw.ps.shaderIndex = 0;

		draw.ps.constantBufferCount = 1;
		draw.ps.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * draw.ps.constantBufferCount
			);
		draw.ps.constantBuffers[0].slotOrRegister = 2;
		draw.ps.constantBuffers[0].indexCount = 1;
		draw.ps.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.ps.constantBuffers[0].indices[0] = 2;

		draw.ps.srvCount = 1;
		draw.ps.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * draw.ps.srvCount
			);
		draw.ps.srvs[0].slotOrRegister = 0;
		draw.ps.srvs[0].indexCount = 1;
		draw.ps.srvs[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.ps.srvs[0].indices[0] = 1;

		draw.ps.samplerCount = 1;
		draw.ps.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * draw.ps.samplerCount
			);
		draw.ps.samplers[0].slotOrRegister = 0;
		draw.ps.samplers[0].indexCount = 1;
		draw.ps.samplers[0].indices = (uint*)allocs->alloc(sizeof(uint));
		draw.ps.samplers[0].indices[0] = 0;

		draw.drawType = DX11DrawPipelineDependancy::DrawType::DRAW_INDEXED;
		draw.argsAsDraw.drawIndexedArgs.indexCount =
			g_ExternalResources.geometryChunks[draw.input.geometryIndex].indexCount;
		draw.argsAsDraw.drawIndexedArgs.startIndexLocation = 0;
		draw.argsAsDraw.drawIndexedArgs.baseVertexLocation = 0;
	}

	return S_OK;
}