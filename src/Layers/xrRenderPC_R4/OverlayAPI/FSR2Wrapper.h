#pragma once
#include <host/ffx_fsr3.h>
#include <host/backends/dx11/ffx_dx11.h>

class Fsr2Wrapper
{
public:
    struct ContextParameters
    {
        uint32_t flags = 0;
        FfxDimensions2D maxRenderSize = { 0, 0 };
        FfxDimensions2D displaySize = { 0, 0 };
        FfxFsr3UpscalerMessage fpMessage;
        ID3D11Device* device = nullptr;
    };

    struct DrawParameters
    {
        ID3D11DeviceContext* deviceContext = nullptr;

        // Inputs
        ID3DResource* unresolvedColorResource = nullptr;
        ID3DResource* unresolvedOpaqueResource = nullptr;

        ID3DResource* motionvectorResource = nullptr;
        ID3DResource* depthbufferResource = nullptr;
        ID3DResource* reactiveMapResource = nullptr;
        ID3DResource* transparencyAndCompositionResource = nullptr;

        // Output
        ID3DResource* resolvedColorResource = nullptr;

        // Arguments
        uint32_t renderWidth = 0;
        uint32_t renderHeight = 0;

        bool cameraReset = false;
        float cameraJitterX = 0.f;
        float cameraJitterY = 0.f;

        bool enableSharpening = true;
        float sharpness = 0.f;

        float frameTimeDelta = 0.f;

        float nearPlane = 1.f;
        float farPlane = 10.f;
        float fovH = 90.f;
    };

public:
    void Create(ContextParameters params);
    void Destroy();

    bool Draw(const DrawParameters& params);

    bool IsCreated() const { return m_created; }
    FfxDimensions2D GetDisplaySize() const { return m_contextDesc.displaySize; }

    ~Fsr2Wrapper();

private:
    bool m_created = false;

    FfxFsr3Context m_context;
    FfxFsr3UpscalerContext m_UpscalerContext;
    FfxFsr3ContextDescription m_contextDesc;
    FfxFsr3UpscalerContextDescription m_UpscalercontextDesc;
    ContextParameters m_contextParams;

    xr_vector<char> m_scratchBuffer;
};

extern Fsr2Wrapper g_Fsr2Wrapper;