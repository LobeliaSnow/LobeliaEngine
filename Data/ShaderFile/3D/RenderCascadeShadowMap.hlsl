#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"

//深度バッファ用
struct ShadowOut {
	float4 pos :SV_POSITION;
	float4 depth :DEPTH;
	float2 tex :TEXCOORD0;
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};

cbuffer ShadowInfo : register(b10) {
	//シャドウマップ作成用
	column_major float4x4 lightViewProj :packoffset(c0.x);
	float4 lightPos :packoffset(c4.x);
	float4 lightDir :packoffset(c5.x);
	//影を付けるか否か
	int useShadowMap : packoffset(c6.x);
	int useVariance : packoffset(c6.y);
	int splitCount : packoffset(c6.z);
	int nowIndex : packoffset(c6.w);
};

//アニメーション用
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

//深度バッファ作成
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
	//出力先のレンダーターゲットを指定
	output.renderTargetIndex = nowIndex;
	return output;
}
//深度マップを作成
float4 CreateShadowMapPS(ShadowOut input) :SV_Target{
	//αテスト ある程度αがなければ影は落ちない
	float alpha = txDiffuse.Sample(samLinear, input.tex).a;
	clip(alpha - 0.9f);
	float4 depth = input.depth.z / input.depth.w;
	//大事！
	//xが深度の期待値で、yが深度の期待値の2乗
	//この値がぼかされて周辺の二乗平均値となり、チェビシェフの不等式に使えるようになる
	if (useVariance)depth.y = depth.x*depth.x;
	depth.a = 1.0f;
	return depth;
}
//カスケード描画されたシャドウマップ
Texture2DArray shadowMaps :register(t0);
//シャドウマップを描画するため
float4 RenderShadowMapPS(PS_IN_TEX ps_in) :SV_Target{
	return shadowMaps.Sample(samLinear,float3(ps_in.tex,nowIndex));
}

//レンダーターゲット配列用のガウスフィルタ