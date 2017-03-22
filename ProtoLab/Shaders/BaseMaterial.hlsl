#include "Inc/Common.hlsli"
#include "Inc/GBuffer.hlsli"
#include "Inc/Utils.hlsli"

// HACK - bindless
#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t16, numDescriptors=unbounded))" \
RootSigEnd


struct VSInput
{
	float3 Position;
	float3 Normal;
	float2 Texcoord;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD;
};

struct View
{
	float4x4 mModelViewProj;
	float4x4 mInverseTransposeModel;
};

struct BaseMaterial
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Transmittance;
	float4 Emission;
	float4 Shininess;
	float4 Ior;
	float4 Dissolve;
    // HACK - bindless
    int4 DiffuseTexId;
};

HLSLConstantBuffer(View, 0, g_View);
HLSLConstantBuffer(BaseMaterial, 1, g_Material);

StructuredBuffer<VSInput> g_VertexArray : register(t0);
Texture2D<float4> g_AllTextures[] : register(t16);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_View.mModelViewProj);
	Out.Normal = mul(float4(In.Normal, 0), g_View.mInverseTransposeModel).xyz;
	Out.Texcoord = In.Texcoord;

	return Out;
}

RootSigDeclaration
GBufferOutput PSMain(VSOutput In)
{
	GBuffer gbuffer;

    // HACK - bindless
	gbuffer.Diffuse = g_AllTextures[g_Material.DiffuseTexId.x].Sample(g_StaticAnisoWrapSampler, In.Texcoord).xyz;

	gbuffer.Specular = IorToF0_Dielectric(g_Material.Ior.x).xxx;
	gbuffer.Normal = In.Normal;
	gbuffer.Roughness = saturate((100.0f - g_Material.Shininess.x) / 100.0f);

	return GBufferEncode(gbuffer);
}