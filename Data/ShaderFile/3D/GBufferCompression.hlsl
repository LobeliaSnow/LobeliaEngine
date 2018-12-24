#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../2D/CodecGBuffer.hlsli"
#include "../Define.h"
#include "ApplyCascadeShadow.hlsli"

//聞きたいこと
//GBufferの詳細
//リソースの管理方法
//最適化
//PostEffect
//Shader,Materialの管理
//マルチスレッド

//リソースは基本JSON→実行時バイナリ
//並列のタイミングは優先度、スロットを設定してそのスレッドで動かす
//Update、LateUpdateが存在して、座標決定しているのはUpdate-LateUpdate
//当たり判定用のメッシュを

//参考サイト
//http://d.hatena.ne.jp/hanecci/20130818/p1
//http://aras-p.info/texts/CompactNormalStorage.html

//定数バッファ
cbuffer SSAO :register(b7) {
	//チェックする深度の範囲
	float offsetPerPixel : packoffset(c0.x);
	//AO使うか否か
	int useAO : packoffset(c0.y);
}

//G-Buffer圧縮用のテストシェーダーです
//出力構造体
struct MRTOutput {
	uint4 data0 : SV_Target0;
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
//static const column_major float4x4 UVTRANS_MATRIX = float4x4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
//接空間へ変換するための行列を作成
inline float4x4 TangentMatrix(in float3 tangent, in float3 binormal, in float3 normal) {
	float4x4 mat = {
		{ float4(tangent, 0.0f) },
		{ float4(binormal, 0.0f) },
		{ float4(normal, 0.0f) },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	return mat;
}
//接空間から戻すための行列を作成
inline float4x4 InverseTangentMatrix(in float3 tangent, in float3 binormal, in float3 normal) {
	//正規直行系なので、転置すれば逆行列となる
	return transpose(TangentMatrix(tangent, binormal, normal));
}
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

//----------------------------------------------------------------------------------------------------------------------
//
//		Entry Point
//
//----------------------------------------------------------------------------------------------------------------------
//G-Buffer作成
//書き込み用GBuffer
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
		//偽従法線生成
		output.binormal = float4(cross(normalize(float3(0.001f, 1.0f, 0.001f)), output.normal.xyz), 0.0f);
		//偽接線生成
		output.tangent = float4(cross(output.normal, output.binormal.xyz), 0.0f);
	}
	output.tex = vs_in.tex;
	return output;
}

//GBuffer作成
//現在 R色G法線B深度
MRTOutput CreateGBufferPS(GBufferPS_IN ps_in) {
	MRTOutput output = (MRTOutput)0;
	//とりあえず適当、TODO : ブレンドステートよく考える
	float4 color = txDiffuse.Sample(samLinear, ps_in.tex);
	clip(color.a - 0.99f);
	//色のエンコード
	output.data0.r = EncodeSDRColor(color);
	//法線マップ
	float4 normal = (float4)0.0f;
	if (useNormalMap) {
		normal = txNormal.Sample(samLinear, ps_in.tex);
		normal = (2.0f * normal - 1.0f);
		//接空間からワールド空間に引っ張る
		normal = mul(normal, TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal));
		normal = normalize(normal);
	}
	else normal = normalize(ps_in.normal);
	//normal = mul(normal, view);
	//normal /= normal.w;
	//normal.xyz = normalize(normal.xyz);
	output.data0.g = EncodeNormalVector(normal, view);
	//深度の保存。位置とかはこれから復元できる
	output.data0.b = EncodeDepth(ps_in.depth.z / ps_in.depth.w);
	output.data0.a = asuint(f32tof16(normal.z) | f32tof16(specularFactor) << 16);
	//エミッションの書き込み
	if (emissionFactor > 0.0f) {
		float4 emission = txEmission.Sample(samLinear, ps_in.tex);
		output.data1.r = EncodeSDRColor(emission);
		output.data1.g = f32tof16(emissionFactor);
	}
	output.data1.xyz = asuint((normal.xyz + 1.0f) / 2.0f);
	return output;
}
//R色、G法線XY、B深度、A現在無し
Texture2D<uint4> txData0 : register(t0);
//R深度
Texture2D<uint4> txData1 : register(t1);
//カスケード描画されたシャドウマップ
Texture2DArray shadowMaps :register(t2);
Texture2D txSplitLVP : register(t3);
//このピクセルのカスケードのインデックスを調べる
int CheckCascadeIndex(in Texture2D tx_data, in int split_count, in float light_space_length) {
	int index = 0;
	for (; index < splitCount; index++) {
		float splitPos = txSplitLVP.Load(int3(4, index, 0)).r;
		if (light_space_length <= splitPos) break;
	}
	return index;
}
//カスケード情報テクスチャからLVP行列を取得
column_major float4x4 LoadCascadeLVP(in Texture2D tx_data, in int index) {
	column_major float4x4 lvp = (float4x4)0.0f;
	//LVP復元
	lvp._11_21_31_41 = tx_data.Load(int3(0, index, 0));
	lvp._12_22_32_42 = tx_data.Load(int3(1, index, 0));
	lvp._13_23_33_43 = tx_data.Load(int3(2, index, 0));
	lvp._14_24_34_44 = tx_data.Load(int3(3, index, 0));
	return lvp;
}
//影をフェッチするためのUVや、比較用の位置を算出
void ComputeShadowLightInfo(in float4x4 lvp, in float4 world_pos, out float depth, out float4 light_uv) {
	//ライト空間での位置を算出
	float4 lightSpacePos = mul(world_pos, lvp);
	light_uv = mul(lightSpacePos, UVTRANS_MATRIX);
	light_uv /= light_uv.w;
	depth = lightSpacePos.z / lightSpacePos.w;
}
//シャドウに履かせる下駄の大きさを算出
float ComputeDepthBias(in float light_depth, in float depth) {
	//最大深度傾斜を求める
	float maxDepthSlope = max(abs(ddx(depth)), abs(ddy(depth)));
	//固定バイアス
	const float bias = 0.003f;
	//深度傾斜
	const float slopedScaleBias = 0.005f;
	//深度クランプ値
	const float depthBiasClamp = 0.1f;
	//アクネ対策の補正値算出
	float shadowAcneBias = bias + slopedScaleBias * maxDepthSlope;
	return min(shadowAcneBias, depthBiasClamp);
}
//シャドウの適用
float ApplyShadow(in float2 light_uv, in float light_depth, in float depth) {
	float shadowBias = 1.0f;
	//範囲外チェック
	if (light_uv.x < 0.0f || light_uv.x > 1.0f || light_uv.y < 0.0f || light_uv.y > 1.0f)return 1.0f;
	//深度判定
	if (depth > light_depth + ComputeDepthBias(light_depth, depth)) shadowBias = 0.3f;
	return shadowBias;
}
//バリアンスシャドウの適用
float ApplyVarianceShadow(in float2 light_uv, in float2 variance_info, in float depth) {
	//範囲外チェック
	if (light_uv.x < 0.0f || light_uv.x > 1.0f || light_uv.y < 0.0f || light_uv.y > 1.0f) return 1.0f;
	//チェビシェフの不等式
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
	//ライトをビュー空間にもっていって空間を合わせる
	output.viewLightDirection = mul(lightDirection.xyz, view);
	output.tex = vs_in.tex;
	return output;
}
//実際に情報をデコードしてライティングを行う
float4 DeferredPS(PS_IN_DEFERRED ps_in) :SV_Target{
	//圧縮された情報の読み込み
	uint4 data0 = LoadUintTexture(txData0, ps_in.tex);
	uint4 data1 = LoadUintTexture(txData1, ps_in.tex);
	//色のデコード
	float4 color = DecodeSDRColor(data0.r);
	color.rgb = pow(color.rgb, 2.2f);
	//法線のデコード
	float4 normal = DecodeNormalVector(data0.g);
	//return float4(normal.xyz, 1.0f);
	//位置のデコード
	float depth = DecodeDepth(data0.b);
	float4 worldPos = DecodeDepthToWorldPos(depth, ps_in.tex, inverseViewProjection);
	float4 viewProjPos = mul(mul(worldPos, view), projection);
	float lightSpaceLength = viewProjPos.w;
	//ライティング処理
	float lambert = saturate(dot(ps_in.viewLightDirection, normal));
	lambert = lambert * 0.5f + 0.5f;
	lambert = lambert * lambert;
	//return float4((float3)lambert, 1.0f);
	color.rgb *= lambert;
	//制御しやすいように0-1反転
	//lambert = lambert * lambert - 1.0f;
	//反転されたものを復元
	//color.rgb *= saturate(lambert * f16tof32(asfloat(data0.a)) + 1.0f);
	if (useAO) {
		float ao = txAO.Sample(samLinear, ps_in.tex);
		color.rgb *= ao;
	}
	if (useShadowMap) {
		//カスケードシャドウ
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
	//スぺキュラの計算
	float4 eyeVector = normalize(cpos - worldPos);
	float4 reflectVector = reflect(-eyeVector, normal);
	color += pow(saturate(dot(reflectVector, eyeVector)), 4)*asfloat(data0.a >> 16);
	color.rgb = pow(color.rgb, 1.0f / 2.2f);
	return color;
}