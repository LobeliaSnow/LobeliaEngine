#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../Define.h"

#define _DEBUG

//ワールド位置
Texture2D txDeferredPos : register(t0);
//ワールド法線
Texture2D txDeferredNormal : register(t1);
//ディフューズ色
Texture2D txDeferredColor : register(t2);
//ビュー空間での位置
Texture2D txDeferredViewPos : register(t3);
//影つけ
Texture2D txShadow : register(t4);
//アンビエントオクルージョン項
Texture2D txDeferredAO : register(t5);
//深度バッファ
Texture2D txLightSpaceDepthMap0 : register(t6);
//SamplerComparisonState samComparsionLinear :register(s1);

//深度バッファ用
struct ShadowOut {
	float4 pos :SV_POSITION;
	float4 depth :DEPTH;
	float2 tex :TEXCOORD0;
};
//MRT用構造体
struct GBufferPS_IN {
	float4 pos : SV_POSITION;
	float4 worldPos : WPOSITION0;
	//法線マップ用
	float4 normal : NORMAL0;
	float4 tangent : NORMAL1;
	float4 binormal : NORMAL2;
	//テクスチャ用
	float2 tex : TEXCOORD0;
	//ビュー空間での位置
	float4 viewPos : TEXCOORD1;
	//影用
	float4 lightTex: TEXCOORD2;
	float4 lightViewPos : LVPOSITION0;
};
struct MRTOutput {
	float4 pos : SV_Target0;
	float4 normal : SV_Target1;
	float4 color : SV_Target2;
	float4 viewPos : SV_Target3;
	float4 shadow : SV_Target4;
};

//ポイントライト用定数バッファ
cbuffer PointLights : register(b6) {
	float4 pointLightPos[LIGHT_SUM];
	float4 pointLightColor[LIGHT_SUM];
	//２次減衰係数
	//xだけ使用 floatで宣言すると16byteアライメントでfloat4でパックされてしまう
	float4 pointLightAttenuation[LIGHT_SUM];
	//有効なポイントライトの数
	int usedLightCount;
};
cbuffer SSAO :register(b7) {
	float offsetPerPixel : packoffset(c0.x);
	int useAO : packoffset(c0.y);
};
cbuffer DeferredOption : register(b8) {
	int useNormalMap : packoffset(c0.x);
	int useSpecularMap : packoffset(c0.y);
};
//9番はガウスで使用
cbuffer ShadowInfo : register(b10) {
	//影用
	column_major float4x4 lightView :packoffset(c0.x);
	column_major float4x4 lightProj :packoffset(c4.x);
	//影を付けるか否か
	int useShadowMap : packoffset(c8.x);
	int useVariance : packoffset(c8.y);
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

//深度バッファ作成
ShadowOut CreateShadowMapVS(VS_IN vs_in) {
	ShadowOut output = (ShadowOut)0;
	output.pos = mul(vs_in.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);
	output.depth = output.pos;
	output.tex = vs_in.tex;
	return output;
}
//G-Buffer作成
GBufferPS_IN CreateGBufferVS(VS_IN vs_in) {
	GBufferPS_IN vs_out = (GBufferPS_IN)0;
	vs_out.pos = mul(vs_in.pos, world);
	vs_out.worldPos = vs_out.pos;
	vs_out.pos = mul(vs_out.pos, view);
	//ビュー空間位置
	//vs_out.viewPos = vs_out.pos.z;
	vs_out.pos = mul(vs_out.pos, projection);
	vs_out.normal = mul(float4(vs_in.normal.xyz, 0), world);
	if (useNormalMap) {
		//偽従法線生成
		vs_out.binormal = float4(cross(normalize(float3(0.0f, 1.0f, 0.1f)), vs_out.normal.xyz), 0.0f);
		//偽接線生成
		vs_out.tangent = float4(cross(vs_out.normal, vs_out.binormal.xyz), 0.0f);
	}
	vs_out.viewPos = vs_out.pos;

	vs_out.tex = vs_in.tex;
	//影生成
	if (useShadowMap) {
		//ライト空間位置へ変換
		column_major float4x4 wlp = mul(lightView, lightProj);
		vs_out.lightViewPos = mul(vs_out.worldPos, wlp);
		vs_out.lightTex = mul(vs_out.lightViewPos, UVTRANS_MATRIX);
	}
	return vs_out;
}

//深度マップを作成
float4 CreateShadowMapPS(ShadowOut input) :SV_Target{
	//αテスト ある程度αがあればシャドウが落ちない
	float alpha = txDiffuse.Sample(samLinear, input.tex).a;
	clip(alpha - 0.9f);
	float4 depth = input.depth.z / input.depth.w;
	depth.a = 1.0f;
	return depth;
}
//GBuffer作成
MRTOutput CreateGBufferPS(GBufferPS_IN ps_in) {
	MRTOutput output = (MRTOutput)0;
	output.pos = ps_in.worldPos;
	output.color = txDiffuse.Sample(samLinear, ps_in.tex);
	//法線マップ
	if (useNormalMap) {
		float4 normalMap = txNormal.Sample(samLinear, ps_in.tex);
		normalMap = (2.0f * normalMap - 1.0f);
		//接空間からワールド空間に引っ張る
		normalMap = mul(normalMap, TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal));
		//エンコード
		output.normal = normalMap * 0.5f + 0.5f;
	}
	else output.normal = ps_in.normal * 0.5f + 0.5f;
	//影生成
	//とりあえず何のひねりもない単純なもの
	if (useShadowMap) {
		float4 shadowTex = ps_in.lightTex / ps_in.lightTex.w;
		if (useVariance) {
			float w = 1.0f / shadowTex.w;
		}
		//最大深度傾斜を求める
		float maxDepthSlope = max(abs(ddx(shadowTex.z)), abs(ddy(shadowTex.z)));
		//固定バイアス
		const float bias = 0.01f;
		//深度傾斜
		const float slopedScaleBias = 0.01f;
		//深度クランプ値
		const float depthBiasClamp = 0.1f;
		float shadowBias = bias + slopedScaleBias * maxDepthSlope;
		//SampleCmpLevelZeroこれ使うべき?
		//float threshold = txLightSpaceDepthMap0.SampleCmpLevelZero(samComparsionLinear, shadowTex.xy, lightSpaceLength + min(shadowBias, depthBiasClamp));
		//output.shadow = float4((float3)lerp(float3(0.34f, 0.34f, 0.34f), float3(1.0f, 1.0f, 1.0f), threshold), 1.0f);
		float lightDepth = txLightSpaceDepthMap0.Sample(samLinear, shadowTex.xy).x;
		float lightSpaceLength = ps_in.lightViewPos.z / ps_in.lightViewPos.w;
		if (lightSpaceLength > lightDepth + min(shadowBias, depthBiasClamp)) {
			output.shadow = float4((float3)1.0f / 2.0f, 1.0f);
		}
		else output.shadow = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//else output.shadow = float4(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef _DEBUG
	//デバッグ表示用
	output.normal.a = 1.0f;
#endif
	output.viewPos = ps_in.viewPos;
	output.viewPos.a = 1.0f;
	return output;
}

//G-Bufferを用いた最小シェーディング
float4 SimpleDeferredPS(PS_IN_TEX ps_in) :SV_Target{
	float4 pos = txDeferredPos.Sample(samLinear, ps_in.tex);
	//デコード
	float4 normal = txDeferredNormal.Sample(samLinear, ps_in.tex) * 2.0 - 1.0;
	normal.a = 0.0f;
	float4 color = txDeferredColor.Sample(samLinear, ps_in.tex);
	// float4 depth = txDeferredDepth.Sample(samLinear, ps_in.tex);
	float lambert = saturate(dot(normalize(cpos - pos),normal));
	color.rgb *= lambert;
	return color;
}
//ポイントライトの色を返す
//eye_vectorは正規化されていないもの
float3 PointLightShading(int index, float3 pos, float3 normal) {
	float3 lightVector = pointLightPos[index] - pos;
	//if (dot(lightVector, normal) < 0)return 0;
	float dist = length(lightVector);
	lightVector = normalize(lightVector);
	//減衰開始
	float attenuation = pow(saturate(pointLightAttenuation[index].x / dist), 4);
	// float attenuation = 1.0f / (dist + pointLightAttenuation[index] * dist * dist);
	return pointLightColor[index] * attenuation;
}
//実際のシェーディングを行う部分
float4 FullDeferredPS(PS_IN_TEX ps_in) :SV_Target{
	float4 pos = txDeferredPos.Sample(samLinear, ps_in.tex);
	//デコード
	float4 normal = txDeferredNormal.Sample(samLinear, ps_in.tex) * 2.0 - 1.0;
	normal.a = 0.0f;
	float4 color = txDeferredColor.Sample(samLinear, ps_in.tex);
	//環境光
	float3 eyeVector = normalize(cpos - pos);
	float lambert = saturate(dot(eyeVector,normal));
	color.rgb *= lambert;
	//アンビエントオクルージョン
	if (useAO) {
		float ao = txDeferredAO.Sample(samLinear, ps_in.tex).x;
		ao = saturate(ao);
		color.rgb *= ao;
	}
	//影
	if (useShadowMap) color.rgb *= txShadow.Sample(samLinear, ps_in.tex);
	//最適化などはまだしていない
	float4 lightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[unroll]//attribute
	for (int i = 0; i < usedLightCount; i++) {
		//照らしているかのチェック
		if (length(pointLightPos[i] - pos) < pointLightAttenuation[i].x * 4.0f) {
			color.rgb += PointLightShading(i, pos.xyz, normal.xyz);
		}
	}
	return color;
}