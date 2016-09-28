#pragma once

class DX12Device;

class DX12RootSignature
{
public:
	DX12RootSignature(ComPtr<ID3D12RootSignature> rootSig);
	~DX12RootSignature();

	ID3D12RootSignature* GetSignature() const { return m_RootSig.Get(); }

private:
	ComPtr<ID3D12RootSignature> m_RootSig;
};

class DX12RootSignatureCompiler
{
public:
	DX12RootSignatureCompiler();
	~DX12RootSignatureCompiler();

	void Begin(uint32_t numParams, uint32_t numStaticSamplers);

	void SetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags);
	void UnsetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags);

	void InitStaticSampler(uint32_t shaderRegister, const D3D12_SAMPLER_DESC& staticSamplerDesc, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_PARAMETER& operator[](uint32_t idx)
	{
		assert(idx < m_RootParams.size());
		return m_RootParams[idx];
	}

	void End();

	std::shared_ptr<DX12RootSignature> Compile(DX12Device* device);

private:
	enum CompilerState
	{
		StateUnknow,
		StateBegin,
		StateEnd,
		StateCompiled,
	};

	CompilerState m_State;

	D3D12_ROOT_SIGNATURE_FLAGS m_Flags;
	uint32_t m_NumInitializedStaticSamplers;

	std::vector<CD3DX12_ROOT_PARAMETER> m_RootParams;
	std::vector<CD3DX12_STATIC_SAMPLER_DESC> m_StaticSamplers;
};