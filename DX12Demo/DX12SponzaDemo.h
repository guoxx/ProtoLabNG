#pragma once

#include "DXSample.h"
#include "DX12/DX12.h"

class Scene;

class DX12SponzaDemo : public DXSample
{
	using super = DXSample;

public:
	DX12SponzaDemo(uint32_t width, uint32_t height, std::wstring name);

	virtual void OnInit(GFX_WHND hwnd) override final;
	virtual void OnUpdate(DX::StepTimer const& timer) override final;
	virtual void OnRender() override final;
	virtual void OnFlip() override final;
	virtual void OnDestroy() override final;

#ifdef _XBOX_ONE
	virtual void OnSuspending() override final;
	virtual void OnResuming() override final;
#endif

	void DrawScene();

	// Rendering helpers
	void Clear();
	void Present();

private:
	void CreateDevice();
	void CreateResources();
	void LoadAssets();

	DX12GraphicManager*									m_GraphicManager;

	std::shared_ptr<Scene>								m_Scene;
};
