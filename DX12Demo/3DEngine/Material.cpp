#include "pch.h"
#include "Material.h"

#include "../DX12/DX12.h"
#include "RenderContext.h"

#include "../../Shaders/CompiledShaders/BaseMaterial_VS.h"
#include "../../Shaders/CompiledShaders/BaseMaterial_PS.h"
#include "../../Shaders/CompiledShaders/BaseMaterial_DepthOnly_VS.h"


Material::Material()
{
}

Material::~Material()
{
}

void Material::Load(DX12GraphicContext* pGfxContext)
{
	CD3DX12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(D3D12_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_NullDescriptorHandle = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DX12GraphicManager::GetInstance()->GetDevice()->CreateShaderResourceView(nullptr, &nullSrvDesc, m_NullDescriptorHandle.GetCpuHandle());

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_GBuffer;

		D3D12_DESCRIPTOR_RANGE descriptorRanges[] = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
		};

		DX12RootSignatureCompiler sigCompiler;
		sigCompiler.Begin(4, 1);
		sigCompiler.End();
		sigCompiler[0].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		sigCompiler[1].InitAsConstantBufferView(0);
		sigCompiler[2].InitAsConstantBufferView(1);
		sigCompiler[3].InitAsDescriptorTable(_countof(descriptorRanges), descriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL);
		CD3DX12_STATIC_SAMPLER_DESC staticSampDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
		staticSampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		sigCompiler.InitStaticSampler(staticSampDesc);
		m_RootSig[shadingCfg] = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

		DX12GraphicPsoCompiler psoCompiler;
		psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_BaseMaterial_VS, sizeof(g_BaseMaterial_VS));
		psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_BaseMaterial_PS, sizeof(g_BaseMaterial_PS));
		psoCompiler.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM);
		psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);

		CD3DX12_RASTERIZER_DESC rasterizerDesc{ D3D12_DEFAULT };
		rasterizerDesc.FrontCounterClockwise = true;
		psoCompiler.SetRasterizerState(rasterizerDesc);

		m_PSO[shadingCfg] = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());
	}

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_DepthOnly;

		DX12RootSignatureCompiler sigCompiler;
		sigCompiler.Begin(2, 0);
		sigCompiler.End();
		sigCompiler[0].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		sigCompiler[1].InitAsConstantBufferView(0);
		m_RootSig[shadingCfg] = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

		DX12GraphicPsoCompiler psoCompiler;
		psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_BaseMaterial_DepthOnly_VS, sizeof(g_BaseMaterial_DepthOnly_VS));
		psoCompiler.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);

		CD3DX12_RASTERIZER_DESC rasterizerDesc{ D3D12_DEFAULT };
		rasterizerDesc.FrontCounterClockwise = true;
		psoCompiler.SetRasterizerState(rasterizerDesc);

		m_PSO[shadingCfg] = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());
	}

	if (!m_AmbientTexName.empty())
	{
		m_AmbientTexture = LoadTexture(pGfxContext, m_AmbientTexName);
	}
	if (!m_DiffuseTexName.empty())
	{
		m_DiffuseTexture = LoadTexture(pGfxContext, m_DiffuseTexName);
	}
	if (!m_SpecularTexName.empty())
	{
		m_SpecularTexture = LoadTexture(pGfxContext, m_SpecularTexName);
	}
	if (!m_SpecularHighlightTexName.empty())
	{
		m_SpecularHighlightTexture = LoadTexture(pGfxContext, m_SpecularHighlightTexName);
	}
	if (!m_BumpTexName.empty())
	{
		m_BumpTexture = LoadTexture(pGfxContext, m_BumpTexName);
	}
	if (!m_DisplacementTexName.empty())
	{
		m_DisplacementTexture = LoadTexture(pGfxContext, m_DisplacementTexName);
	}
	if (!m_AlphaTexName.empty())
	{
		m_AlphaTexture = LoadTexture(pGfxContext, m_AlphaTexName);
	}
}

std::shared_ptr<DX12Texture> Material::LoadTexture(DX12GraphicContext* pGfxContext, std::string texname)
{
	std::shared_ptr<DX12Texture> tex = std::shared_ptr<DX12Texture>();
	tex.reset(DX12Texture::LoadTGAFromFile(DX12GraphicManager::GetInstance()->GetDevice(), pGfxContext, texname.c_str()));
	return tex;	
}

void Material::Apply(RenderContext* pRenderContext, DX12GraphicContext* pGfxContext)
{
	ShadingConfiguration shadingCfg = pRenderContext->GetShadingCfg();

	pGfxContext->SetGraphicsRootSignature(m_RootSig[shadingCfg]);

	pGfxContext->SetPipelineState(m_PSO[shadingCfg].get());

	if (shadingCfg == ShadingConfiguration_GBuffer)
	{
		ConstantBuffer(View)
		{
			float4x4 mModelViewProj;
			float4x4 mInverseTransposeModel;
		};

		ConstantBuffer(BaseMaterial)
		{
			float4 Ambient;
			float4 Diffuse;
			float4 Specular;
			float4 Transmittance;
			float4 Emission;
			float4 Shininess;
			float4 Ior;
			float4 Dissolve;
		};

		DirectX::XMMATRIX mModel = pRenderContext->GetModelMatrix();
		DirectX::XMMATRIX mInverseModel = DirectX::XMMatrixInverse(nullptr, mModel);
		DirectX::XMMATRIX mInverseTransposeModel = DirectX::XMMatrixTranspose(mInverseModel);
		View view;
		DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrix()));
		DirectX::XMStoreFloat4x4(&view.mInverseTransposeModel, mInverseTransposeModel);

		BaseMaterial baseMaterial;
		baseMaterial.Ambient = DirectX::XMFLOAT4{ m_Ambient.x, m_Ambient.y, m_Ambient.z, 0.0f };
		baseMaterial.Diffuse = DirectX::XMFLOAT4{ m_Diffuse.x, m_Diffuse.y, m_Diffuse.z, 0.0f };
		baseMaterial.Specular = DirectX::XMFLOAT4{ m_Specular.x, m_Specular.y, m_Specular.z, 0.0f };
		baseMaterial.Transmittance = DirectX::XMFLOAT4{ m_Transmittance.x, m_Transmittance.y, m_Transmittance.z, 0.0f };
		baseMaterial.Emission = DirectX::XMFLOAT4{ m_Emission.x, m_Emission.y, m_Emission.z, 0.0f };
		baseMaterial.Shininess = DirectX::XMFLOAT4{ m_Shininess, m_Shininess, m_Shininess, m_Shininess };
		baseMaterial.Ior = DirectX::XMFLOAT4{ m_Ior, m_Ior, m_Ior, m_Ior };
		baseMaterial.Dissolve = DirectX::XMFLOAT4{ m_Dissolve, m_Dissolve, m_Dissolve, m_Dissolve };

		pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &view, sizeof(view));
		pGfxContext->SetGraphicsRootDynamicConstantBufferView(2, &baseMaterial, sizeof(baseMaterial));

		if (m_DiffuseTexture.get() == nullptr)
		{
			pGfxContext->SetGraphicsRootDescriptorTable(3, m_NullDescriptorHandle);
		}
		else
		{
			pGfxContext->SetGraphicsRootDescriptorTable(3, m_DiffuseTexture->GetSRV());
		}
	}
	else if (shadingCfg == ShadingConfiguration_DepthOnly)
	{
		ConstantBuffer(View)
		{
			float4x4 mModelViewProj;
		};

		View view;
		DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrix()));

		pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &view, sizeof(view));
	}
}
