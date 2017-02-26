#include "pch.h"
#include "PointLightFilter2D.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/PointLight.h"

#include "../../Shaders/CompiledShaders/PointLight.h"


PointLightFilter2D::PointLightFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicsContextAutoExecutor executor;
	DX12GraphicsContext* pGfxContext = executor.GetGraphicsContext();

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(2);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler.InitDescriptorTable(1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler.SetupDescriptorRange(1, 0, CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 0});
	m_RootSig = sigCompiler.Compile(device);

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_PointLight_VS, sizeof(g_PointLight_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_PointLight_PS, sizeof(g_PointLight_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetBlendState(CD3DX12::BlendAdditive());
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(device);
}

PointLightFilter2D::~PointLightFilter2D()
{
}

void PointLightFilter2D::Apply(DX12GraphicsContext * pGfxContext, const RenderContext* pRenderContext, const PointLight* pPointLight)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	struct Constants
	{
		float4x4 mInvView;
		float4x4 mInvProj;
		float4 CameraPosition;

		float4 LightPosition;
		float4 LightIntensity;
		float4 LightRadius;
		float4x4 mLightViewProj[6];
	};
	Constants constants;

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	DirectX::XMStoreFloat4x4(&constants.mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.mInvProj, DirectX::XMMatrixTranspose(mInvProj));
	DirectX::XMStoreFloat4(&constants.CameraPosition, pRenderContext->GetCamera()->GetTranslation());

	DirectX::XMStoreFloat4(&constants.LightPosition, pPointLight->GetTranslation());
	constants.LightIntensity = pPointLight->GetIntensity();
	constants.LightRadius = DirectX::XMFLOAT4{ pPointLight->GetRadiusStart(), pPointLight->GetRadiusEnd(), 0, 0 };
	for (int i = 0; i < 6; ++i)
	{
		DirectX::XMMATRIX mLightView;
		DirectX::XMMATRIX mLightProj;
		pPointLight->GetViewAndProjMatrix(nullptr, (PointLight::AXIS)i, DX12PointLightShadowMapSize, &mLightView, &mLightProj);
		DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
		DirectX::XMStoreFloat4x4(&constants.mLightViewProj[i], DirectX::XMMatrixTranspose(mLightViewProj));
	}

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void PointLightFilter2D::Draw(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}