#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../Define.h"

#define _DEBUG

//最終的にはリンケージにてある程度は解決できるようにしたい

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
//カスケード用
Texture2D txLightSpaceDepthMap0 : register(t6);
Texture2D txLightSpaceDepthMap1 : register(t7);
Texture2D txLightSpaceDepthMap2 : register(t8);
Texture2D txLightSpaceDepthMap3 : register(t9);
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
#ifdef CASCADE
	float lightLength : TEXCOORD2;
	float4 lightTex[4]: TEXCOORD3;
	float4 lightViewPos[4] : LVPOSITION0;
#else
	float4 lightTex: TEXCOORD2;
	float4 lightViewPos : LVPOSITION0;
#endif
};
//出力構造体
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
	column_major float4x4 lightProj0 :packoffset(c4.x);
#ifdef CASCADE
	//テクスチャ配列で送ってやれば可変長にできる
	//とりあえずテスト用に4で作成
	//それか、カスケードの最大枚数を決めてそれから範囲内の値を使うのも手
	column_major float4x4 lightProj1 :packoffset(c8.x);
	column_major float4x4 lightProj2 :packoffset(c12.x);
	column_major float4x4 lightProj3 :packoffset(c16.x);
	float4 lightPos :packoffset(c20.x);
	float4 lightDir :packoffset(c21.x);
	float splitPos0 : packoffset(c22.x);
	float splitPos1 : packoffset(c22.y);
	float splitPos2 : packoffset(c22.z);
	float splitPos3 : packoffset(c22.w);
	//影を付けるか否か
	int useShadowMap : packoffset(c23.x);
	int useVariance : packoffset(c23.y);
#else
	//影を付けるか否か
	int useShadowMap : packoffset(c8.x);
	int useVariance : packoffset(c8.y);
#endif
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
#ifdef CASCADE
		//ライト空間位置へ変換 0
		vs_out.worldPos /= vs_out.worldPos.w;
		column_major float4x4 wlp = mul(lightView, lightProj0);
		vs_out.lightViewPos[0] = mul(vs_out.worldPos, wlp);
		vs_out.lightTex[0] = mul(vs_out.lightViewPos[0], UVTRANS_MATRIX);
		//1
		wlp = mul(lightView, lightProj1);
		vs_out.lightViewPos[1] = mul(vs_out.worldPos, wlp);
		vs_out.lightTex[1] = mul(vs_out.lightViewPos[1], UVTRANS_MATRIX);
		//2
		wlp = mul(lightView, lightProj2);
		vs_out.lightViewPos[2] = mul(vs_out.worldPos, wlp);
		vs_out.lightTex[2] = mul(vs_out.lightViewPos[2], UVTRANS_MATRIX);
		//3
		wlp = mul(lightView, lightProj3);
		vs_out.lightViewPos[3] = mul(vs_out.worldPos, wlp);
		vs_out.lightTex[3] = mul(vs_out.lightViewPos[3], UVTRANS_MATRIX);
		//長さ
		vs_out.lightLength = length(lightPos.xyz - vs_out.worldPos.xyz);
#else
		//ライト空間位置へ変換
		column_major float4x4 wlp = mul(lightView, lightProj0);
		vs_out.lightViewPos = mul(vs_out.worldPos, wlp);
		vs_out.lightTex = mul(vs_out.lightViewPos, UVTRANS_MATRIX);
#endif
	}
	return vs_out;
}

//深度マップを作成
float4 CreateShadowMapPS(ShadowOut input) :SV_Target{
	//αテスト ある程度αがあればシャドウが落ちない
	float alpha = txDiffuse.Sample(samLinear, input.tex).a;
	clip(alpha - 0.9f);
	float4 depth = input.depth.z / input.depth.w;
	//大事！
	//xが深度の期待値で、yが深度の期待値の2乗
	//この値がぼかされて周辺の平均値となり、チェビシェフの不等式に使えるようになる
	if (useVariance)depth.y = depth.x*depth.x;
	depth.a = 1.0f;
	return depth;
}
#ifdef CASCADE
//とりあえず今は適当にカスケード対応
//C++側のソース含め、シェーダーとの兼ね合いで作り直す
//カスケード用
float4 CascadeVarianceShadow(GBufferPS_IN ps_in) {
	//現在の描画対象の距離算出
	float lightSpaceLength = ps_in.lightLength;
	float2 lightDepth = (float2)0.0f;
	float4 shadowTex = (float4)0;
	if (lightSpaceLength < splitPos0) {
		shadowTex = ps_in.lightTex[0] / ps_in.lightTex[0].w;
		lightDepth = txLightSpaceDepthMap0.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[0].z / ps_in.lightViewPos[0].w;
	}
	else if (lightSpaceLength < splitPos1) {
		shadowTex = ps_in.lightTex[1] / ps_in.lightTex[1].w;
		lightDepth = txLightSpaceDepthMap1.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[1].z / ps_in.lightViewPos[1].w;
	}
	else if (lightSpaceLength < splitPos2) {
		shadowTex = ps_in.lightTex[2] / ps_in.lightTex[2].w;
		lightDepth = txLightSpaceDepthMap2.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[2].z / ps_in.lightViewPos[2].w;
	}
	else {
		shadowTex = ps_in.lightTex[3] / ps_in.lightTex[3].w;
		lightDepth = txLightSpaceDepthMap3.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[3].z / ps_in.lightViewPos[3].w;
	}
	if (shadowTex.x < 0.0f || shadowTex.x > 1.0f || shadowTex.y < 0.0f || shadowTex.y > 1.0f) {
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (lightSpaceLength - 0.01f > lightDepth.x) {
		//チェビシェフの不等式
		float variance = lightDepth.y - (lightDepth.x * lightDepth.x);
		//variance = min(1.0f, max(0.0f, variance + 0.01f));
		float md = lightSpaceLength - lightDepth.x;
		float p = variance / (variance + (md*md));
		float shadow = max(p, lightSpaceLength <= lightDepth.x);
		shadow = saturate(shadow * 0.5f + 0.5f);
		return float4((float3)shadow, 1.0f);
	}
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
float4 CascadeShadow(GBufferPS_IN ps_in) {
	//現在の描画対象の距離算出
	float lightSpaceLength = ps_in.lightLength;
	float2 lightDepth = (float2)0.0f;
	float4 shadowTex = (float4)0.0f;
	if (lightSpaceLength < splitPos0) {
		shadowTex = ps_in.lightTex[0] / ps_in.lightTex[0].w;
		lightDepth = txLightSpaceDepthMap0.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[0].z / ps_in.lightViewPos[0].w;
	}
	else if (lightSpaceLength < splitPos1) {
		shadowTex = ps_in.lightTex[1] / ps_in.lightTex[1].w;
		lightDepth = txLightSpaceDepthMap1.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[1].z / ps_in.lightViewPos[1].w;
	}
	else if (lightSpaceLength < splitPos2) {
		shadowTex = ps_in.lightTex[2] / ps_in.lightTex[2].w;
		lightDepth = txLightSpaceDepthMap2.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[2].z / ps_in.lightViewPos[2].w;
	}
	else {
		shadowTex = ps_in.lightTex[3] / ps_in.lightTex[3].w;
		lightDepth = txLightSpaceDepthMap3.Sample(samLinear, shadowTex.xy).rg;
		lightSpaceLength = ps_in.lightViewPos[3].z / ps_in.lightViewPos[3].w;
	}
	//最大深度傾斜を求める
	float maxDepthSlope = max(abs(ddx(shadowTex.z)), abs(ddy(shadowTex.z)));
	//固定バイアス
	const float bias = 0.01f;
	//深度傾斜
	const float slopedScaleBias = 0.01f;
	//深度クランプ値
	const float depthBiasClamp = 0.1f;
	//アクネ対策の補正値算出
	float shadowBias = bias + slopedScaleBias * maxDepthSlope;
	//深度判定
	if (lightSpaceLength > lightDepth.x + min(shadowBias, depthBiasClamp)) return float4((float3)1.0f / 2.0f, 1.0f);
	else return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
#else
	//バリアンスシャドウマップ
	float4 VarianceShadow(float4 shadowTex, GBufferPS_IN ps_in) {
	//現在の描画対象の距離算出
	float lightSpaceLength = ps_in.lightViewPos.z / ps_in.lightViewPos.w;
	//カメラ位置からのZ値を見て、遮蔽物の深度値取得(無ければ上と同じものが入る)
	float2 lightDepth = txLightSpaceDepthMap0.Sample(samLinear, shadowTex.xy).rg;
	//参考
	//http://maverickproj.web.fc2.com/d3d11_28.html
	//https://riyaaaaasan.hatenablog.com/entry/2018/03/15/215910
	//特にお世話になった シャドウマップ作成時にyに二乗データを入れないといけない。
	//http://marupeke296.com/cgi-bin/cbbs/cbbs.cgi?mode=al2&namber=2399&rev=&no=0&P=R&KLOG=3
	if (lightSpaceLength - 0.01f > lightDepth.x) {
		//チェビシェフの不等式
		float variance = lightDepth.y - (lightDepth.x * lightDepth.x);
		//variance = min(1.0f, max(0.0f, variance + 0.01f));
		float md = lightSpaceLength - lightDepth.x;
		float p = variance / (variance + (md*md));
		float shadow = max(p, lightSpaceLength <= lightDepth.x);
		shadow = saturate(shadow * 0.5f + 0.5f);
		return float4((float3)shadow, 1.0f);
	}
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
//通常のシャドウマップ
float4 Shadow(float4 shadowTex, GBufferPS_IN ps_in) {
	//現在の描画対象の距離算出
	float lightSpaceLength = ps_in.lightViewPos.z / ps_in.lightViewPos.w;
	//カメラ位置からのZ値を見て、遮蔽物の深度値取得(無ければ上と同じものが入る)
	float2 lightDepth = txLightSpaceDepthMap0.Sample(samLinear, shadowTex.xy).rg;
	//SampleCmpLevelZero使うべき?
	//Slope Scale Depth Bias
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
	//最大深度傾斜を求める
	float maxDepthSlope = max(abs(ddx(shadowTex.z)), abs(ddy(shadowTex.z)));
	//固定バイアス
	const float bias = 0.01f;
	//深度傾斜
	const float slopedScaleBias = 0.01f;
	//深度クランプ値
	const float depthBiasClamp = 0.1f;
	//アクネ対策の補正値算出
	float shadowBias = bias + slopedScaleBias * maxDepthSlope;
	//深度判定
	if (lightSpaceLength > lightDepth.x + min(shadowBias, depthBiasClamp)) return float4((float3)1.0f / 2.0f, 1.0f);
	else return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
#endif
//GBuffer作成
//TODO : 各所関数化
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
	if (useShadowMap) {
#ifdef CASCADE
		//分散シャドウマップ
		if (useVariance) output.shadow = CascadeVarianceShadow(ps_in);
		//何のひねりもない単純なもの
		else output.shadow = CascadeShadow(ps_in);
#else
		//シャドウマップサンプル先を求める
		float4 shadowTex = ps_in.lightTex / ps_in.lightTex.w;
		//シャドウマップの範囲外は影なし
		if (shadowTex.x < 0.0f || shadowTex.x > 1.0f || shadowTex.y < 0.0f || shadowTex.y > 1.0f) {
			output.shadow = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			//現在の描画対象の距離算出
			float lightSpaceLength = ps_in.lightViewPos.z / ps_in.lightViewPos.w;
			//カメラ位置からのZ値を見て、遮蔽物の深度値取得(無ければ上と同じものが入る)
			float2 lightDepth = txLightSpaceDepthMap0.Sample(samLinear, shadowTex.xy).rg;
			//分散シャドウマップ
			if (useVariance) output.shadow = VarianceShadow(shadowTex, ps_in);
			//何のひねりもない単純なもの
			else output.shadow = Shadow(shadowTex, ps_in);
		}
#endif
	}
	//else output.shadow = float4(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef _DEBUG
	//デバッグ表示用
	output.normal.a = 1.0f;
#endif
	//現状SSAO用 深度値のみ使用している形だが、あればほかのことにも使えそうなので全て書き出し
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
		float ao = txDeferredAO.Sample(samLinear, ps_in.tex).r;
		ao = saturate(ao);
		color.rgb *= ao;
	}
	//影 この段階ではもうバリアンスとかは考慮の必要がない
	if (useShadowMap) color.rgb *= txShadow.Sample(samLinear, ps_in.tex).r;
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