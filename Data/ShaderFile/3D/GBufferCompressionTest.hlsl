#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../Define.h"

//G-Buffer圧縮用のテストシェーダーです
//出力構造体
struct MRTOutput {
	//R色
	uint4 data0 : SV_Target0;
	//とりあえずおいている
	uint4 data1 : SV_Target1;
};

//MRT用構造体
struct GBufferPS_IN {
	float4 pos : SV_POSITION;
	float4 depth : DEPTH0;
	//法線マップ用
	float4 normal : NORMAL0;
	float4 tangent : NORMAL1;
	float4 binormal : NORMAL2;
	//テクスチャ用
	float2 tex : TEXCOORD0;
	//ビュー空間での位置
	float4 viewPos : TEXCOORD1;
};

//定数
//単位行列
static const column_major float4x4 IDENTITY_MATRIX = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
//影用
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

//アニメーション用
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
//G-Buffer作成
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
		//偽従法線生成
		output.binormal = float4(cross(normalize(float3(0.001f, 1.0f, 0.001f)), output.normal.xyz), 0.0f);
		//偽接線生成
		output.tangent = float4(cross(output.normal, output.binormal.xyz), 0.0f);
	}
	output.tex = vs_in.tex;
	return output;
}
//GBuffer作成
//TODO : 各所関数化
MRTOutput CreateGBufferPS(GBufferPS_IN ps_in) {
	MRTOutput output = (MRTOutput)0;
	uint4 color = (uint4)(txDiffuse.Sample(samLinear, ps_in.tex)*255.0f);
	//とりあえず適当、TODO : ブレンドステートよく考える
	clip(color.a - 0.99f);
	//色のエンコード
	output.data0.r = color.r | color.g << 8 | color.b << 16 | color.a << 24;
	//法線マップ
	float4 normal = (float4)0.0f;
	if (useNormalMap) {
		float4 normalMap = txNormal.Sample(samLinear, ps_in.tex);
		normalMap = (2.0f * normalMap - 1.0f);
		//接空間からワールド空間に引っ張る
		normalMap = mul(normalMap, TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal));
		//エンコード
		normal = normalMap * 0.5f + 0.5f;
	}
	else normal = ps_in.normal * 0.5f + 0.5f;
	//エミッションカラー取得
	float4 emission = txEmission.Sample(samLinear, ps_in.tex);
	//法線のエンコード
	output.data0.g = asuint(f32tof16(normal.x) | f32tof16(normal.y) << 16);
	output.data0.b = asuint(f32tof16(normal.z));
	//深度の保存。位置とかはこれから復元できる
	//ここは精度が欲しいため、f16へは圧縮しない
	output.data0.a = asuint(ps_in.depth.z / ps_in.depth.w);
	return output;
}

//RG色,BA法線
Texture2D<uint4> txData0 : register(t0);
//R深度
Texture2D<uint4> txData1 : register(t1);

float4 DeferredPS(PS_IN_TEX ps_in) :SV_Target {
	//圧縮された情報の読み込み
	float2	pixelSize;
	txData0.GetDimensions(pixelSize.x, pixelSize.y);
	uint4 data0 = txData0.Load(uint3(ps_in.tex*pixelSize, 0));
	uint4 data1 = txData1.Load(uint3(ps_in.tex*pixelSize, 0));
	//色のデコード
	uint4 colorUint = uint4(data0.r & 255, (data0.r >> 8) & 255, (data0.r >> 16) & 255, data0.r >> 24);
	float4 color = colorUint / 255.0f;
	//法線のデコード
	//http://aras-p.info/texts/CompactNormalStorage.html#method01xy
	float4 normal = (float4)0.0f;
	normal.x = asfloat(f16tof32(data0.g));
	normal.y = asfloat(f16tof32(data0.g >> 16));
	normal.z = asfloat(f16tof32(data0.b));
	normal.xyz = normal.xyz * 2.0 - 1.0;
	//位置のデコード
	float4 viewProjPos = float4(ps_in.tex.x*2.0f - 1.0f,(1.0f - ps_in.tex.y)*2.0f - 1.0f, asfloat(data0.a), 1.0f);
	float4 worldPos = mul(viewProjPos, inverseViewProjection);
	worldPos /= worldPos.w;
	//ライティング処理
	float lambert = saturate(dot(lightDirection, (normal)));
	lambert = lambert * 0.5f + 0.5f;
	lambert = lambert * lambert;
	//return float4((float3)lambert, 1.0f);
	color.rgb *= lambert;
	return color;
}