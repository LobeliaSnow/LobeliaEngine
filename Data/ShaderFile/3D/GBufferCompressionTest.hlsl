#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../Define.h"

//G-Buffer���k�p�̃e�X�g�V�F�[�_�[�ł�
//�o�͍\����
struct MRTOutput {
	//R�F
	uint4 data0 : SV_Target0;
	//�Ƃ肠���������Ă���
	uint4 data1 : SV_Target1;
};

//MRT�p�\����
struct GBufferPS_IN {
	float4 pos : SV_POSITION;
	float4 depth : DEPTH0;
	//�@���}�b�v�p
	float4 normal : NORMAL0;
	float4 tangent : NORMAL1;
	float4 binormal : NORMAL2;
	//�e�N�X�`���p
	float2 tex : TEXCOORD0;
	//�r���[��Ԃł̈ʒu
	float4 viewPos : TEXCOORD1;
};

//�萔
//�P�ʍs��
static const column_major float4x4 IDENTITY_MATRIX = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
//�e�p
static const column_major float4x4 UVTRANS_MATRIX = float4x4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

inline float4x4 TangentMatrix(float3 tangent, float3 binormal, float3 normal) {
	float4x4 mat =
	{
		{ float4(tangent, 0.0f) },
		{ float4(binormal, 0.0f) },
		{ float4(normal, 0.0f) },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	return mat;
}

//�A�j���[�V�����p
inline column_major float4x4 FetchBoneMatrix(uint iBone) { return keyFrames[iBone]; }
inline Skin SkinVertex(VS_IN vs_in) {
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
//G-Buffer�쐬
GBufferPS_IN CreateGBufferVS(VS_IN vs_in) {
	GBufferPS_IN output = (GBufferPS_IN)0;
	if (useAnimation) {
		Skin skinned = SkinVertex(vs_in);
		output.pos = mul(skinned.pos, world);
		output.normal = normalize(mul(float4(skinned.normal.xyz, 0), world));
	}
	else {
		output.pos = mul(vs_in.pos, world);
		output.normal = mul(float4(vs_in.normal.xyz, 0), world);
	}
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);
	output.depth = output.pos;
	if (useNormalMap) {
		//�U�]�@������
		output.binormal = float4(cross(normalize(float3(0.001f, 1.0f, 0.001f)), output.normal.xyz), 0.0f);
		//�U�ڐ�����
		output.tangent = float4(cross(output.normal, output.binormal.xyz), 0.0f);
	}
	output.tex = vs_in.tex;
	return output;
}
//GBuffer�쐬
//TODO : �e���֐���
MRTOutput CreateGBufferPS(GBufferPS_IN ps_in) {
	MRTOutput output = (MRTOutput)0;
	uint4 color = (uint4)(txDiffuse.Sample(samLinear, ps_in.tex)*255.0f);
	//�Ƃ肠�����K���ATODO : �u�����h�X�e�[�g�悭�l����
	clip(color.a - 0.99f);
	//�F�̃G���R�[�h
	output.data0.r = color.r | color.g << 8 | color.b << 16 | color.a << 24;
	//�@���}�b�v
	float4 normal = (float4)0.0f;
	if (useNormalMap) {
		float4 normalMap = txNormal.Sample(samLinear, ps_in.tex);
		normalMap = (2.0f * normalMap - 1.0f);
		//�ڋ�Ԃ��烏�[���h��ԂɈ�������
		normalMap = mul(normalMap, TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal));
		//�G���R�[�h
		normal = normalMap * 0.5f + 0.5f;
	}
	else normal = ps_in.normal * 0.5f + 0.5f;
	//�G�~�b�V�����J���[�擾
	float4 emission = txEmission.Sample(samLinear, ps_in.tex);
	//�@���̃G���R�[�h
	output.data0.g = asuint(f32tof16(normal.x) | f32tof16(normal.y) << 16);
	output.data0.b = asuint(f32tof16(normal.z));
	//�[�x�̕ۑ��B�ʒu�Ƃ��͂��ꂩ�畜���ł���
	//�����͐��x���~�������߁Af16�ւ͈��k���Ȃ�
	output.data0.a = asuint(ps_in.depth.z / ps_in.depth.w);
	return output;
}

//RG�F,BA�@��
Texture2D<uint4> txData0 : register(t0);
//R�[�x
Texture2D<uint4> txData1 : register(t1);

float4 DeferredPS(PS_IN_TEX ps_in) :SV_Target {
	//���k���ꂽ���̓ǂݍ���
	float2	pixelSize;
	txData0.GetDimensions(pixelSize.x, pixelSize.y);
	uint4 data0 = txData0.Load(uint3(ps_in.tex*pixelSize, 0));
	uint4 data1 = txData1.Load(uint3(ps_in.tex*pixelSize, 0));
	//�F�̃f�R�[�h
	uint4 colorUint = uint4(data0.r & 255, (data0.r >> 8) & 255, (data0.r >> 16) & 255, data0.r >> 24);
	float4 color = colorUint / 255.0f;
	//�@���̃f�R�[�h
	//http://aras-p.info/texts/CompactNormalStorage.html#method01xy
	float4 normal = (float4)0.0f;
	normal.x = asfloat(f16tof32(data0.g));
	normal.y = asfloat(f16tof32(data0.g >> 16));
	normal.z = asfloat(f16tof32(data0.b));
	normal.xyz = normal.xyz * 2.0 - 1.0;
	//�ʒu�̃f�R�[�h
	float4 viewProjPos = float4(ps_in.tex.x*2.0f - 1.0f,(1.0f - ps_in.tex.y)*2.0f - 1.0f, asfloat(data0.a), 1.0f);
	float4 worldPos = mul(viewProjPos, inverseViewProjection);
	worldPos /= worldPos.w;
	//���C�e�B���O����
	float lambert = saturate(dot(lightDirection, (normal)));
	lambert = lambert * 0.5f + 0.5f;
	lambert = lambert * lambert;
	//return float4((float3)lambert, 1.0f);
	color.rgb *= lambert;
	return color;
}