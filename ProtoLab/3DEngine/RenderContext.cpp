#include "pch.h"
#include "RenderContext.h"


RenderContext::RenderContext()
{
	m_ShadingCfg = ShadingConfiguration_GBuffer;
}

RenderContext::~RenderContext()
{
}

DX12ColorSurface * RenderContext::AcquireRSMRadiantIntensitySurfaceForDirectionalLight(DirectionalLight * pDirLight)
{
	RenderableSurfaceHandle<DX12ColorSurface> handle;

	auto result = m_RSMRadiantIntensitySurfaceForDirLights.find(pDirLight);
	if (result != m_RSMRadiantIntensitySurfaceForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc);
		m_RSMRadiantIntensitySurfaceForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle.Get();
}

DX12ColorSurface * RenderContext::AcquireRSMNormalSurfaceForDirectionalLight(DirectionalLight * pDirLight)
{
	RenderableSurfaceHandle<DX12ColorSurface> handle;

	auto result = m_RSMNormalSurfaceForDirLights.find(pDirLight);
	if (result != m_RSMNormalSurfaceForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc);
		m_RSMNormalSurfaceForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle.Get();
}

DX12ColorSurface * RenderContext::AcquireEVSMSurfaceForDirectionalLight(DirectionalLight * pDirLight)
{
	RenderableSurfaceHandle<DX12ColorSurface> handle;

	auto result = m_EVSMSurfaceForDirLights.find(pDirLight);
	if (result != m_EVSMSurfaceForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R32G32B32A32_FLOAT, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc);
		m_EVSMSurfaceForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle.Get();
}

DX12DepthSurface*  RenderContext::AcquireDepthSurfaceForDirectionalLight(DirectionalLight* pDirLight)
{
	RenderableSurfaceHandle<DX12DepthSurface> handle;

	auto result = m_ShadowMapForDirLights.find(pDirLight);
	if (result != m_ShadowMapForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_D32_FLOAT, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc);
		m_ShadowMapForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle.Get();
}

std::array<DX12ColorSurface*, 6> RenderContext::AcquireEVSMSurfaceForPointLight(PointLight * pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> handles;

	auto result = m_EVSMSurfaceForPointLights.find(pPointLight);
	if (result != m_EVSMSurfaceForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R32G32B32A32_FLOAT, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
		};
		m_EVSMSurfaceForPointLights.insert(std::make_pair(pPointLight, handles));
	}

	return std::array<DX12ColorSurface*, 6>{
			handles[0].Get(),
			handles[1].Get(),
			handles[2].Get(),
			handles[3].Get(),
			handles[4].Get(),
			handles[5].Get(),
	};
}

std::array<DX12ColorSurface*, 6> RenderContext::AcquireRSMRadiantIntensitySurfaceForPointLight(PointLight * pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> handles;

	auto result = m_RSMRadiantIntensitySurfaceForPointLights.find(pPointLight);
	if (result != m_RSMRadiantIntensitySurfaceForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
		};
		m_RSMRadiantIntensitySurfaceForPointLights.insert(std::make_pair(pPointLight, handles));
	}

	return std::array<DX12ColorSurface*, 6>{
			handles[0].Get(),
			handles[1].Get(),
			handles[2].Get(),
			handles[3].Get(),
			handles[4].Get(),
			handles[5].Get(),
	};
}

std::array<DX12ColorSurface*, 6> RenderContext::AcquireRSMNormalSurfaceForPointLight(PointLight * pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> handles;

	auto result = m_RSMNormalSurfaceForPointLights.find(pPointLight);
	if (result != m_RSMNormalSurfaceForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
		};
		m_RSMNormalSurfaceForPointLights.insert(std::make_pair(pPointLight, handles));
	}

	return std::array<DX12ColorSurface*, 6>{
			handles[0].Get(),
			handles[1].Get(),
			handles[2].Get(),
			handles[3].Get(),
			handles[4].Get(),
			handles[5].Get(),
	};
}

std::array<DX12DepthSurface*, 6> RenderContext::AcquireDepthSurfaceForPointLight(PointLight* pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6> handles;

	auto result = m_ShadowMapForPointLights.find(pPointLight);
	if (result != m_ShadowMapForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_D32_FLOAT, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
		};
		m_ShadowMapForPointLights.insert(std::make_pair(pPointLight, handles));
	}

	return std::array<DX12DepthSurface*, 6>{
			handles[0].Get(),
			handles[1].Get(),
			handles[2].Get(),
			handles[3].Get(),
			handles[4].Get(),
			handles[5].Get(),
	};
}

void RenderContext::ReleaseDepthSurfacesForAllLights()
{
	// TODO
}