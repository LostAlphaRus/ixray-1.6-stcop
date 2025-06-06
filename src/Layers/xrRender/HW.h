// HW.h: interface for the CHW class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "HWCaps.h"

#ifndef _EDITOR
#	include <renderdoc/api/app/renderdoc_app.h>
#endif

#ifndef _MAYA_EXPORT
#include "stats_manager.h"
#endif

struct SDL_Window;

#ifdef USE_DX11
#define RContext ((ID3D11DeviceContext*)Device.GetRenderContext())
#define RDevice ((ID3D11Device*)Device.GetRenderDevice())
#define RSwapchainTarget ((ID3D11RenderTargetView*)Device.GetSwapchainTexture())
#define RTarget ((ID3D11RenderTargetView*)Device.GetRenderTexture())
#define RDepth ((ID3D11DepthStencilView*)Device.GetDepthTexture())
#define RSwapchain ((IDXGISwapChain*)Device.GetSwapchain())
#else
#define RContext ((IDirect3DDevice9*)Device.GetRenderContext())
#define RDevice ((IDirect3DDevice9*)Device.GetRenderDevice())
#define RSwapchainTarget ((IDirect3DSurface9*)Device.GetSwapchainTexture())
#define RTarget ((IDirect3DSurface9*)Device.GetRenderTexture())
#define RDepth ((IDirect3DSurface9*)Device.GetDepthTexture())
#define RSwapchain ((IDirect3DDevice9*)Device.GetSwapchain())
#endif

#define RFeatureLevel Device.GetFeatureLevel()

#ifdef IXR_WINDOWS
#if defined(D3D12_SDK_VERSION)
using IXRRenderDevice = ID3D12Device;
using IXRRenderDeviceContext = ID3D12GraphicsCommandList;
#elif defined(D3D11_SDK_VERSION)
using IXRRenderDevice = ID3D11Device;
using IXRRenderDeviceContext = ID3D11DeviceContext;
#elif defined(D3D10_SDK_VERSION)
using IXRRenderDevice = ID3D10Device;
using IXRRenderDeviceContext = IUnknown;
#elif defined(DIRECT3D_VERSION) && DIRECT3D_VERSION >= 0x0900
using IXRRenderDevice = IDirect3DDevice9;
// since we don't have the context interface at all just make it unknown
using IXRRenderDeviceContext = IUnknown;
#else
#error unknown DirectX SDK
#endif
#endif