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