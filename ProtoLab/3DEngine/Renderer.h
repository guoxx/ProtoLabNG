#pragma once

#include "../DX12/DX12.h"

#include "SurfaceSet.h"
#include "RenderContext.h"
#include "RenderableSurfaceManager.h"
#include "Pass/PointLightShadingPass.h"
#include "Pass/DirectionalLightShadingPass.h"
#include "Pass/ToneMappingPass.h"

class ILight;
class Scene;
class Camera;
class Filter2D;
class ConvertEVSMFilter2D;
class AntiAliasingFilter2D;
class ResolveToSwapChainFilter2D;
class LightCullingPass;
class TiledShadingPass;
class ComputeProcessing;

class Renderer
{
public:
	Renderer(GFX_HWND hwnd, int32_t width, int32_t height);
	~Renderer();

	void Render(const Camera* pCamera, Scene* pScene);

	void Flip();

private:
	void RenderShadowMaps(const Camera* pCamera, Scene* pScene);

	void DeferredLighting(const Camera* pCamera, Scene* pScene);

	void RenderGBuffer(const Camera* pCamera, Scene* pScene);

    void PostProcess(const Camera* pCamera, Scene* pScene);

	void ResolveToSwapChain();

	void RenderDebugMenu();

    void AAFilter();

    void CalcAvgLuminance(DX12ColorSurface* surf);

    void ToneMap();

	int32_t m_Width;
	int32_t m_Height;
    int32_t m_FrameIdx;

	RenderContext m_RenderContext;	

    GBufferSurfaceSet m_GBuffer;

    PostProcessSurfaceSet m_PostProcessSurfaces;

	RenderableSurfaceHandle<DX12ColorSurface> m_PostProcessSurface;
	RenderableSurfaceHandle<DX12ColorSurface> m_HistoryLightingSurface;

	std::shared_ptr<DX12SwapChain> m_SwapChain;

	std::shared_ptr<ConvertEVSMFilter2D> m_ConvertEVSMFilter2D;
	std::shared_ptr<AntiAliasingFilter2D> m_AntiAliasingFilter2D;

	std::shared_ptr<LightCullingPass> m_LightCullingPass;
	std::shared_ptr<TiledShadingPass> m_TiledShadingPass;
	std::shared_ptr<DX12StructuredBuffer> m_AllPointLights;
	std::shared_ptr<DX12StructuredBuffer> m_AllDirectionalLights;
	std::shared_ptr<DX12StructuredBuffer> m_VisiblePointLights;

    std::shared_ptr<PointLightShadingPass> m_PointLightShadingPass;
    std::shared_ptr<DirectionalLightShadingPass> m_DirectionalLightShadingPass;

    std::shared_ptr<ToneMappingPass> m_ToneMap;

    std::shared_ptr<ImageProcessing> m_CopyFP16;
    std::shared_ptr<ImageProcessing> m_ResolveToSwapChain;

    std::shared_ptr<ComputeProcessing> m_ReduceLuminanceInitial;
    std::shared_ptr<ComputeProcessing> m_ReduceLuminance;
    std::shared_ptr<ComputeProcessing> m_ReduceLuminanceFinal;
};

