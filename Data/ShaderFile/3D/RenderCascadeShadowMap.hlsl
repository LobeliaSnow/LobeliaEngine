#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"

//�[�x�o�b�t�@�p
struct ShadowOut {
	float4 pos :SV_POSITION;
	float4 depth :DEPTH;
	float2 tex :TEXCOORD0;
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};
cbuffer ShadowInfo : register(b10) {
	//�V���h�E�}�b�v�쐬�p
	column_major float4x4 lightViewProj :packoffset(c0.x);
	float4 lightPos :packoffset(c4.x);
	float4 lightDir :packoffset(c5.x);
	//�e��t���邩�ۂ�
	int useShadowMap : packoffset(c6.x);
	int useVariance : packoffset(c6.y);
	int splitCount : packoffset(c6.z);
	int nowIndex : packoffset(c6.w);
};

//�A�j���[�V�����p
inline column_major float4x4 FetchBoneMatrix(in uint iBone) { return keyFrames[iBone]; }
inline Skin SkinVertex(in VS_IN vs_in) {
	Skin output = (Skin)0;
	float4 pos = vs_in.pos;
	float3 normal = vs_in.normal.xyz;
	[unroll]
	for (int i = 0; i < 4; i++) {
		uint bone = vs_in.clusterIndex[i];
		float weight = vs_in.weights[i];
		column_major float4x4 m = FetchBoneMatrix(bone);
		output.pos += weight * mul(pos, m);
		output.normal += weight * mul(normal, (float3x3) m);
	}
	return output;
}

//�[�x�o�b�t�@�쐬
ShadowOut CreateShadowMapVS(VS_IN vs_in) {
	ShadowOut output = (ShadowOut)0;
	if (useAnimation) {
		Skin skinned = SkinVertex(vs_in);
		output.pos = mul(skinned.pos, world);
	}
	else output.pos = mul(vs_in.pos, world);
	output.pos = mul(output.pos, lightViewProj);
	output.depth = output.pos;
	output.tex = vs_in.tex;
	//�o�͐�̃����_�[�^�[�Q�b�g���w��
	output.renderTargetIndex = nowIndex;
	return output;
}
//�[�x�}�b�v���쐬
float4 CreateShadowMapPS(ShadowOut input) :SV_Target{
	//���e�X�g ������x�����Ȃ���Ήe�͗����Ȃ�
	float alpha = txDiffuse.Sample(samLinear, input.tex).a;
	clip(alpha - 0.9f);
	float4 depth = input.depth.z / input.depth.w;
	//�厖�I
	//x���[�x�̊��Ғl�ŁAy���[�x�̊��Ғl��2��
	//���̒l���ڂ�����Ď��ӂ̓�敽�ϒl�ƂȂ�A�`�F�r�V�F�t�̕s�����Ɏg����悤�ɂȂ�
	if (useVariance)depth.y = depth.x*depth.x;
	depth.a = 1.0f;
	return depth;
}
//�J�X�P�[�h�`�悳�ꂽ�V���h�E�}�b�v
Texture2DArray shadowMaps :register(t0);
//�V���h�E�}�b�v��`�悷�邽��
float4 RenderShadowMapPS(PS_IN_TEX ps_in) :SV_Target{
	return shadowMaps.Sample(samLinear,float3(ps_in.tex,nowIndex));
}

//�����_�[�^�[�Q�b�g�z��p�̃K�E�X�t�B���^
struct GAUSSIAN_VS_OUT {
	float4 pos    : SV_POSITION;
	float2 tex0 : TEXCOORD0;
	float2 tex1 : TEXCOORD1;
	float2 tex2 : TEXCOORD2;
	float2 tex3 : TEXCOORD3;
	float2 offset : TEXCOORD8;
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};
cbuffer CascadeGaussianInfo : register(b5) {
	float  weight0 : packoffset(c0.x);
	float  weight1 : packoffset(c0.y);
	float  weight2 : packoffset(c0.z);
	float  weight3 : packoffset(c0.w);
	int texIndex : packoffset(c1.x);
	float texWidth : packoffset(c1.y);
	float texHeight : packoffset(c1.z);
}

Texture2DArray colorArray :register(t0);

GAUSSIAN_VS_OUT GaussianFilterVSX(VS_IN_TEX vs_in) {
	GAUSSIAN_VS_OUT output = (GAUSSIAN_VS_OUT)0;
	output.pos = vs_in.pos;
	output.tex0 = vs_in.tex + float2(-1.0f / texWidth, 0.0f);
	output.tex1 = vs_in.tex + float2(-3.0f / texWidth, 0.0f);
	output.tex2 = vs_in.tex + float2(-5.0f / texWidth, 0.0f);
	output.tex3 = vs_in.tex + float2(-7.0f / texWidth, 0.0f);
	output.offset = float2(8.0f / texWidth, 0.0f);
	//�o�͐�̃����_�[�^�[�Q�b�g���w��
	output.renderTargetIndex = texIndex;
	return output;
}
GAUSSIAN_VS_OUT GaussianFilterVSY(VS_IN_TEX vs_in) {
	GAUSSIAN_VS_OUT output = (GAUSSIAN_VS_OUT)0;
	output.pos = vs_in.pos;
	output.tex0 = vs_in.tex + float2(0.0f, -1.0f / texHeight);
	output.tex1 = vs_in.tex + float2(0.0f, -3.0f / texHeight);
	output.tex2 = vs_in.tex + float2(0.0f, -5.0f / texHeight);
	output.tex3 = vs_in.tex + float2(0.0f, -7.0f / texHeight);
	output.offset = float2(0.0f, 8.0f / texHeight);
	//�o�͐�̃����_�[�^�[�Q�b�g���w��
	output.renderTargetIndex = texIndex;
	return output;
}
float4 GaussianFilterPS(GAUSSIAN_VS_OUT ps_in) :SV_Target{
	float4 ret = (float4)0;
	ret += (colorArray.Sample(samLinear, float3(saturate(ps_in.tex0), texIndex)) + colorArray.Sample(samLinear, float3(saturate(ps_in.tex3 + ps_in.offset),texIndex))) * weight0;
	ret += (colorArray.Sample(samLinear, float3(saturate(ps_in.tex1), texIndex)) + colorArray.Sample(samLinear, float3(saturate(ps_in.tex2 + ps_in.offset),texIndex))) * weight1;
	ret += (colorArray.Sample(samLinear, float3(saturate(ps_in.tex2), texIndex)) + colorArray.Sample(samLinear, float3(saturate(ps_in.tex1 + ps_in.offset),texIndex))) * weight2;
	ret += (colorArray.Sample(samLinear, float3(saturate(ps_in.tex3), texIndex)) + colorArray.Sample(samLinear, float3(saturate(ps_in.tex0 + ps_in.offset),texIndex))) * weight3;
	return ret;
}
