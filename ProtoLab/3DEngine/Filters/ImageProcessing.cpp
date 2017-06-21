#include "pch.h"
#include "ImageProcessing.h"


ImageProcessing::ImageProcessing(DX12Device* device,
                                 const void* pVSBin, uint32_t vsDataSize,
                                 const void* pPSBin, uint32_t psDataSize,
                                 DXGI_FORMAT renderTargetFmt)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12ScopedGraphicsContext pGfxContext;

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

    DX12RootSignatureDeserializer sigDeserialier{pVSBin, sizeof(vsDataSize)};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12GraphicsPsoDesc psoDesc;
	psoDesc.SetShaderFromBin(DX12ShaderTypeVertex, pVSBin, vsDataSize);
	psoDesc.SetShaderFromBin(DX12ShaderTypePixel, pPSBin, psDataSize);
	psoDesc.SetRoogSignature(m_RootSig.get());
	psoDesc.SetRenderTargetFormat(renderTargetFmt);
	psoDesc.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoDesc.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = DX12PsoCompiler::Compile(DX12GraphicsManager::GetInstance()->GetDevice(), &psoDesc);
}

ImageProcessing::~ImageProcessing()
{
}

void ImageProcessing::Apply(DX12GraphicsContext * pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ImageProcessing::Draw(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
