#include "stdafx.h"

#include "FSR2Wrapper.h"

Fsr2Wrapper g_Fsr2Wrapper;

void Fsr2Wrapper::Create(Fsr2Wrapper::ContextParameters params)
{
    if(m_created) {
        Destroy();
    }

    if(RFeatureLevel < D3D_FEATURE_LEVEL_11_0) {
        return;
    }

    m_contextParams = params;

    // Setup DX11 interface.

    m_scratchBuffer.resize(ffxGetScratchMemorySizeDX11(1));
    FfxErrorCode errorCode = ffxGetInterfaceDX11(&m_contextDesc.backendInterfaceUpscaling, params.device, m_scratchBuffer.data(), m_scratchBuffer.size(), 1);
    //errorCode = ffxGetInterfaceDX11(&m_contextDesc.backendInterfaceFrameInterpolation, params.device, m_scratchBuffer.data(), m_scratchBuffer.size(), 1);
    //errorCode = ffxGetInterfaceDX11(&m_contextDesc.backendInterfaceSharedResources, params.device, m_scratchBuffer.data(), m_scratchBuffer.size(), 1);
    errorCode = ffxGetInterfaceDX11(&m_UpscalercontextDesc.backendInterface, params.device, m_scratchBuffer.data(), m_scratchBuffer.size(), 1);
    R_ASSERT(errorCode == FFX_OK);

    // This adds a ref to the device. 
    // The reference will get freed in ffxFsr2ContextDestroy
    //m_contextDesc.device = ffxGetDeviceDX11(params.device);
    m_contextDesc.maxRenderSize = params.maxRenderSize;
    m_contextDesc.maxUpscaleSize = params.displaySize;
    m_contextDesc.displaySize = params.displaySize;

    // You should config the flags you need based on your own project
    m_contextDesc.flags = FFX_FSR3_ENABLE_HIGH_DYNAMIC_RANGE | FFX_FSR3_ENABLE_UPSCALING_ONLY;

    if(params.fpMessage) {
        m_contextDesc.flags |= FFX_FSR3_ENABLE_DEBUG_CHECKING;
        m_contextDesc.fpMessage = params.fpMessage;
    }

    errorCode = ffxFsr3ContextCreate(&m_context, &m_contextDesc);
    R_ASSERT(errorCode == FFX_OK);

    m_UpscalercontextDesc.flags = FfxFsr3UpscalerInitializationFlagBits::FFX_FSR3UPSCALER_ENABLE_HIGH_DYNAMIC_RANGE;
    m_UpscalercontextDesc.fpMessage = params.fpMessage;
    m_UpscalercontextDesc.maxRenderSize = params.maxRenderSize;
    m_UpscalercontextDesc.maxUpscaleSize = params.displaySize;

    errorCode = ffxFsr3UpscalerContextCreate(&m_UpscalerContext, &m_UpscalercontextDesc);
    R_ASSERT(errorCode == FFX_OK);

    m_created = true;
}

void Fsr2Wrapper::Destroy()
{
    if(!m_created) {
        return;
    }

    m_created = false;
    ffxFsr3UpscalerContextDestroy(&m_UpscalerContext);
    ffxFsr3ContextDestroy(&m_context);
}

 FfxResource ffxGetResourceDX11X(ID3D11Resource* dx11Resource,
    FfxResourceDescription                     ffxResDescription,
    wchar_t const* ffxResName,
    FfxResourceStates                          state =FFX_RESOURCE_STATE_COMPUTE_READ)
{
    FfxResource resource = {};
    resource.resource = reinterpret_cast<void*>(const_cast<ID3D11Resource*>(dx11Resource));
    resource.state = state;
    resource.description = ffxResDescription;

#ifdef _DEBUG
    if (ffxResName) {
        wcscpy_s(resource.name, ffxResName);
    }
#endif

    return resource;
}

bool Fsr2Wrapper::Draw(const DrawParameters& params)
{
    if(!m_created) {
        Msg("! Fsr2Wrapper not created. Need use linear filter");
        return false;
    }

    FfxFsr3UpscalerDispatchDescription dispatchParameters = {};
    dispatchParameters.commandList = params.deviceContext;

    dispatchParameters.color = ffxGetResourceDX11X(params.unresolvedColorResource, GetFfxResourceDescriptionDX11(params.unresolvedColorResource), L"FSR3_InputColor");
    dispatchParameters.depth = ffxGetResourceDX11X(params.depthbufferResource, GetFfxResourceDescriptionDX11(params.depthbufferResource), L"FSR3_InputDepth");

    dispatchParameters.motionVectors = ffxGetResourceDX11X(params.motionvectorResource, GetFfxResourceDescriptionDX11(params.motionvectorResource), L"FSR3_InputMotionVectors");
    dispatchParameters.exposure = ffxGetResourceDX11X(nullptr, {}, L"FSR3_InputExposure");

    dispatchParameters.reactive = ffxGetResourceDX11X(params.reactiveMapResource, GetFfxResourceDescriptionDX11(params.reactiveMapResource), L"FSR3_InputReactiveMap");
    dispatchParameters.transparencyAndComposition = ffxGetResourceDX11X(params.transparencyAndCompositionResource, GetFfxResourceDescriptionDX11(params.transparencyAndCompositionResource), L"FSR3_TransparencyAndCompositionMap");

    dispatchParameters.output = ffxGetResourceDX11X(params.resolvedColorResource, GetFfxResourceDescriptionDX11(params.resolvedColorResource), L"FSR3_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);

    dispatchParameters.jitterOffset.x = params.cameraJitterX;
    dispatchParameters.jitterOffset.y = params.cameraJitterY;

    dispatchParameters.motionVectorScale.x = -float(params.renderWidth * 0.5f);
    dispatchParameters.motionVectorScale.y = float(params.renderHeight * 0.5f);

    dispatchParameters.reset = params.cameraReset;

    dispatchParameters.enableSharpening = params.enableSharpening;
    dispatchParameters.sharpness = params.sharpness;

    dispatchParameters.frameTimeDelta = params.frameTimeDelta;

    dispatchParameters.preExposure = 1.0f;

    dispatchParameters.renderSize.width = params.renderWidth;
    dispatchParameters.renderSize.height = params.renderHeight;
    dispatchParameters.upscaleSize = m_UpscalercontextDesc.maxUpscaleSize;

    dispatchParameters.cameraFar = params.farPlane;
    dispatchParameters.cameraNear = params.nearPlane;

    dispatchParameters.cameraFovAngleVertical = params.fovH;

    dispatchParameters.viewSpaceToMetersFactor = 1.0f;

    //dispatchParameters.autoTcThreshold = 0.1f;
    //dispatchParameters.autoTcScale = 1.0f;
    //dispatchParameters.autoReactiveScale = 5.0f;
    //dispatchParameters.autoReactiveMax = 0.9f;
    //
    //// EXPERIMENTAL feature, auto-generate reactive mask
    //// Turn it off if you don't unresolvedOpaqueResource it
    //
    //if(params.unresolvedOpaqueResource) {
    //    dispatchParameters.enableAutoReactive = true;
    //    dispatchParameters.colorOpaqueOnly = ffxGetResourceDX11X(params.unresolvedOpaqueResource, L"FSR2_InputColor");
    //}

    FfxErrorCode errorCode = ffxFsr3UpscalerContextDispatch(&m_UpscalerContext, &dispatchParameters);

    if(errorCode != FFX_OK) {
        Msg("! ffxFsr2ContextDispatch not valid. Need use linear filter");
        return false;
    }

    return true;
}

Fsr2Wrapper::~Fsr2Wrapper()
{
    Destroy();
}
