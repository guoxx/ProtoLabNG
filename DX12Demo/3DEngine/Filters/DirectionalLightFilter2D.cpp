#include "pch.h"
#include "DirectionalLightFilter2D.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/DirectionalLight.h"


namespace
{
	struct Constants
	{
		float4x4 mInvView;
		float4x4 mInvProj;
		float4 LightDirection;
		float4 LightIrradiance;
		float4 CameraPosition;
	};
}

DirectionalLightFilter2D::DirectionalLightFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	DX12GraphicManager::GetInstance()->UpdateBufer(pGfxContext, m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_DESCRIPTOR_RANGE descriptorRanges1[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0 },
	};
	D3D12_DESCRIPTOR_RANGE descriptorRanges2[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, 0 },
	};
	D3D12_DESCRIPTOR_RANGE descriptorRanges3[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, 0 },
	};
	D3D12_DESCRIPTOR_RANGE descriptorRanges4[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, 0 },
	};

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(5, 1);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsDescriptorTable(_countof(descriptorRanges1), descriptorRanges1, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler[2].InitAsDescriptorTable(_countof(descriptorRanges2), descriptorRanges2, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler[3].InitAsDescriptorTable(_countof(descriptorRanges3), descriptorRanges3, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler[4].InitAsDescriptorTable(_countof(descriptorRanges4), descriptorRanges4, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler.InitStaticSampler(CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT));
	m_RootSig = sigCompiler.Compile(device);

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromFile(DX12ShaderTypeVertex, L"DirectionalLight.hlsl", "VSMain");
	psoCompiler.SetShaderFromFile(DX12ShaderTypePixel, L"DirectionalLight.hlsl", "PSMain");
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	m_PSO = psoCompiler.Compile(device);

	m_ConstantsBuffer = std::make_shared<DX12ConstantsBuffer>(device, sizeof(Constants), 0);
}

DirectionalLightFilter2D::~DirectionalLightFilter2D()
{
}

void DirectionalLightFilter2D::Apply(DX12GraphicContext * pGfxContext, const RenderContext* pRenderContext, const DirectionalLight* pLight)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig.get());
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	Constants constants;
	DirectX::XMStoreFloat4x4(&constants.mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.mInvProj, DirectX::XMMatrixTranspose(mInvProj));
	constants.LightDirection = pLight->GetDirection();
	constants.LightIrradiance = pLight->GetIrradiance();
	DirectX::XMStoreFloat4(&constants.CameraPosition, pRenderContext->GetCamera()->GetTranslation());

	pGfxContext->ResourceTransitionBarrier(m_ConstantsBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	DX12GraphicManager::GetInstance()->UpdateBufer(pGfxContext, m_ConstantsBuffer.get(), &constants, sizeof(constants));
	pGfxContext->ResourceTransitionBarrier(m_ConstantsBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void DirectionalLightFilter2D::Draw(DX12GraphicContext* pGfxContext)
{
	pGfxContext->SetGraphicsRootConstantBufferView(0, m_ConstantsBuffer.get());
	pGfxContext->DrawIndexed(6, 0);
}
