#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../2D/CodecGBuffer.hlsli"
#include "../Define.h"
#include "ApplyCascadeShadow.hlsli"

//������������
//GBuffer�̏ڍ�
//���\�[�X�̊Ǘ����@
//�œK��
//PostEffect
//Shader,Material�̊Ǘ�
//�}���`�X���b�h

//���\�[�X�͊�{JSON�����s���o�C�i��
//����̃^�C�~���O�͗D��x�A�X���b�g��ݒ肵�Ă��̃X���b�h�œ�����
//Update�ALateUpdate�����݂��āA���W���肵�Ă���̂�Update-LateUpdate
//�����蔻��p�̃��b�V����

//�Q�l�T�C�g
//http://d.hatena.ne.jp/hanecci/20130818/p1
//http://aras-p.info/texts/CompactNormalStorage.html

//�萔�o�b�t�@
cbuffer SSAO :register(b7) {
	//�`�F�b�N����[�x�͈̔�
	float offsetPerPixel : packoffset(c0.x);
	//AO�g�����ۂ�
	int useAO : packoffset(c0.y);
}

//G-Buffer���k�p�̃e�X�g�V�F�[�_�[�ł�
//�o�͍\����
struct MRTOutput {
	uint4 data0 : SV_Target0;
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
//static const column_major float4x4 UVTRANS_MATRIX = float4x4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
//�ڋ�Ԃ֕ϊ����邽�߂̍s����쐬
inline float4x4 TangentMatrix(in float3 tangent, in float3 binormal, in float3 normal) {
	float4x4 mat = {
		{ float4(tangent, 0.0f) },
		{ float4(binormal, 0.0f) },
		{ float4(normal, 0.0f) },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	return mat;
}
//�ڋ�Ԃ���߂����߂̍s����쐬
inline float4x4 InverseTangentMatrix(in float3 tangent, in float3 binormal, in float3 normal) {
	//���K���s�n�Ȃ̂ŁA�]�u����΋t�s��ƂȂ�
	return transpose(TangentMatrix(tangent, binormal, normal));
}
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

//----------------------------------------------------------------------------------------------------------------------
//
//		Entry Point
//
//----------------------------------------------------------------------------------------------------------------------
//G-Buffer�쐬
//�������ݗpGBuffer
cbuffer DeferredOption : register(b8) {
	float lightingFactor : packoffset(c0.x);
	float specularFactor : packoffset(c0.y);
	float emissionFactor : packoffset(c0.z);
};
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
//���� R�FG�@��B�[�x
MRTOutput CreateGBufferPS(GBufferPS_IN ps_in) {
	MRTOutput output = (MRTOutput)0;
	//�Ƃ肠�����K���ATODO : �u�����h�X�e�[�g�悭�l����
	float4 color = txDiffuse.Sample(samLinear, ps_in.tex);
	clip(color.a - 0.99f);
	//�F�̃G���R�[�h
	output.data0.r = EncodeSDRColor(color);
	//�@���}�b�v
	float4 normal = (float4)0.0f;
	if (useNormalMap) {
		normal = txNormal.Sample(samLinear, ps_in.tex);
		normal = (2.0f * normal - 1.0f);
		//�ڋ�Ԃ��烏�[���h��ԂɈ�������
		normal = mul(normal, TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal));
		normal = normalize(normal);
	}
	else normal = normalize(ps_in.normal);
	//normal = mul(normal, view);
	//normal /= normal.w;
	//normal.xyz = normalize(normal.xyz);
	output.data0.g = EncodeNormalVector(normal, view);
	//�[�x�̕ۑ��B�ʒu�Ƃ��͂��ꂩ�畜���ł���
	output.data0.b = EncodeDepth(ps_in.depth.z / ps_in.depth.w);
	output.data0.a = asuint(f32tof16(normal.z) | f32tof16(specularFactor) << 16);
	//�G�~�b�V�����̏�������
	if (emissionFactor > 0.0f) {
		float4 emission = txEmission.Sample(samLinear, ps_in.tex);
		output.data1.r = EncodeSDRColor(emission);
		output.data1.g = f32tof16(emissionFactor);
	}
	output.data1.xyz = asuint((normal.xyz + 1.0f) / 2.0f);
	return output;
}
//R�F�AG�@��XY�AB�[�x�AA���ݖ���
Texture2D<uint4> txData0 : register(t0);
//R�[�x
Texture2D<uint4> txData1 : register(t1);
//�J�X�P�[�h�`�悳�ꂽ�V���h�E�}�b�v
Texture2DArray shadowMaps :register(t2);
Texture2D txSplitLVP : register(t3);
//���̃s�N�Z���̃J�X�P�[�h�̃C���f�b�N�X�𒲂ׂ�
int CheckCascadeIndex(in Texture2D tx_data, in int split_count, in float light_space_length) {
	int index = 0;
	for (; index < splitCount; index++) {
		float splitPos = txSplitLVP.Load(int3(4, index, 0)).r;
		if (light_space_length <= splitPos) break;
	}
	return index;
}
//�J�X�P�[�h���e�N�X�`������LVP�s����擾
column_major float4x4 LoadCascadeLVP(in Texture2D tx_data, in int index) {
	column_major float4x4 lvp = (float4x4)0.0f;
	//LVP����
	lvp._11_21_31_41 = tx_data.Load(int3(0, index, 0));
	lvp._12_22_32_42 = tx_data.Load(int3(1, index, 0));
	lvp._13_23_33_43 = tx_data.Load(int3(2, index, 0));
	lvp._14_24_34_44 = tx_data.Load(int3(3, index, 0));
	return lvp;
}
//�e���t�F�b�`���邽�߂�UV��A��r�p�̈ʒu���Z�o
void ComputeShadowLightInfo(in float4x4 lvp, in float4 world_pos, out float depth, out float4 light_uv) {
	//���C�g��Ԃł̈ʒu���Z�o
	float4 lightSpacePos = mul(world_pos, lvp);
	light_uv = mul(lightSpacePos, UVTRANS_MATRIX);
	light_uv /= light_uv.w;
	depth = lightSpacePos.z / lightSpacePos.w;
}
//�V���h�E�ɗ������鉺�ʂ̑傫�����Z�o
float ComputeDepthBias(in float light_depth, in float depth) {
	//�ő�[�x�X�΂����߂�
	float maxDepthSlope = max(abs(ddx(depth)), abs(ddy(depth)));
	//�Œ�o�C�A�X
	const float bias = 0.003f;
	//�[�x�X��
	const float slopedScaleBias = 0.005f;
	//�[�x�N�����v�l
	const float depthBiasClamp = 0.1f;
	//�A�N�l�΍�̕␳�l�Z�o
	float shadowAcneBias = bias + slopedScaleBias * maxDepthSlope;
	return min(shadowAcneBias, depthBiasClamp);
}
//�V���h�E�̓K�p
float ApplyShadow(in float2 light_uv, in float light_depth, in float depth) {
	float shadowBias = 1.0f;
	//�͈͊O�`�F�b�N
	if (light_uv.x < 0.0f || light_uv.x > 1.0f || light_uv.y < 0.0f || light_uv.y > 1.0f)return 1.0f;
	//�[�x����
	if (depth > light_depth + ComputeDepthBias(light_depth, depth)) shadowBias = 0.3f;
	return shadowBias;
}
//�o���A���X�V���h�E�̓K�p
float ApplyVarianceShadow(in float2 light_uv, in float2 variance_info, in float depth) {
	//�͈͊O�`�F�b�N
	if (light_uv.x < 0.0f || light_uv.x > 1.0f || light_uv.y < 0.0f || light_uv.y > 1.0f) return 1.0f;
	//�`�F�r�V�F�t�̕s����
	float variance = variance_info.y - (variance_info.x * variance_info.x);
	variance = min(1.0f, max(0.0f, variance + 0.01f));
	float delta = depth - variance_info.x;
	float p = variance / (variance + (delta*delta));
	float shadowBias = max(p, depth <= variance_info.x);
	shadowBias = saturate(shadowBias * 0.7f + 0.3f);
	return shadowBias;
}

Texture2D txAO : register(t4);
struct PS_IN_DEFERRED {
	float4 pos : SV_POSITION;
	float4 viewLightDirection : LIGHT_DIRECTION;
	float2 tex : TEXCOORD;
};
PS_IN_DEFERRED DeferredVS(VS_IN_TEX vs_in) {
	PS_IN_DEFERRED output;
	output.pos = vs_in.pos;
	//���C�g���r���[��Ԃɂ����Ă����ċ�Ԃ����킹��
	output.viewLightDirection = mul(lightDirection.xyz, view);
	output.tex = vs_in.tex;
	return output;
}
//���ۂɏ����f�R�[�h���ă��C�e�B���O���s��
float4 DeferredPS(PS_IN_DEFERRED ps_in) :SV_Target{
	//���k���ꂽ���̓ǂݍ���
	uint4 data0 = LoadUintTexture(txData0, ps_in.tex);
	uint4 data1 = LoadUintTexture(txData1, ps_in.tex);
	//�F�̃f�R�[�h
	float4 color = DecodeSDRColor(data0.r);
	color.rgb = pow(color.rgb, 2.2f);
	//�@���̃f�R�[�h
	float4 normal = DecodeNormalVector(data0.g);
	//return float4(normal.xyz, 1.0f);
	//�ʒu�̃f�R�[�h
	float depth = DecodeDepth(data0.b);
	float4 worldPos = DecodeDepthToWorldPos(depth, ps_in.tex, inverseViewProjection);
	float4 viewProjPos = mul(mul(worldPos, view), projection);
	float lightSpaceLength = viewProjPos.w;
	//���C�e�B���O����
	float lambert = saturate(dot(ps_in.viewLightDirection, normal));
	lambert = lambert * 0.5f + 0.5f;
	lambert = lambert * lambert;
	//return float4((float3)lambert, 1.0f);
	color.rgb *= lambert;
	//���䂵�₷���悤��0-1���]
	//lambert = lambert * lambert - 1.0f;
	//���]���ꂽ���̂𕜌�
	//color.rgb *= saturate(lambert * f16tof32(asfloat(data0.a)) + 1.0f);
	if (useAO) {
		float ao = txAO.Sample(samLinear, ps_in.tex);
		color.rgb *= ao;
	}
	if (useShadowMap) {
		//�J�X�P�[�h�V���h�E
		int index = CheckCascadeIndex(txSplitLVP,splitCount, lightSpaceLength);
		column_major float4x4 lvp = LoadCascadeLVP(txSplitLVP, index);
		float depth = 0.0f; float4 lightUV = (float4)0.0f;
		ComputeShadowLightInfo(lvp, worldPos, depth, lightUV);
		float shadowBias = 1.0f;
		if (useVariance) {
			float2 varianceInfo = shadowMaps.Sample(samLinear, float3(lightUV.xy, index)).rg;
			shadowBias = ApplyVarianceShadow(lightUV, varianceInfo, depth);
		}
		else {
			float lightDepth = shadowMaps.Sample(samLinear, float3(lightUV.xy, index)).r;
			shadowBias = ApplyShadow(lightUV, lightDepth, depth);
		}
		color.rgb *= shadowBias;
	}
	//�X�؃L�����̌v�Z
	float4 eyeVector = normalize(cpos - worldPos);
	float4 reflectVector = reflect(-eyeVector, normal);
	color += pow(saturate(dot(reflectVector, eyeVector)), 4)*asfloat(data0.a >> 16);
	color.rgb = pow(color.rgb, 1.0f / 2.2f);
	return color;
}