#include "pch.h"
#include "Filter2D.h"

#include "../../Shaders/CompiledShaders/IdentityFilter2D.h"


Filter2D::Filter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicsContextAutoExecutor executor;
	DX12GraphicsContext* pGfxContext = executor.GetGraphicsContext();

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

    DX12RootSignatureDeserializer sigDeserialier{g_IdentityFilter2D_VS, sizeof(g_IdentityFilter2D_VS)};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_IdentityFilter2D_VS, sizeof(g_IdentityFilter2D_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_IdentityFilter2D_PS, sizeof(g_IdentityFilter2D_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());
}

Filter2D::~Filter2D()
{
}

void Filter2D::Apply(DX12GraphicsContext * pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Filter2D::Draw(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
