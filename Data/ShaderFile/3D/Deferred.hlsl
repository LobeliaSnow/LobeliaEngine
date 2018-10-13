#include "../Header.hlsli"
#include "Header3D.hlsli"
#include "../2D/Header2D.hlsli"
#include "../Define.h"

#define __DEBUG
//ワールド位置
Texture2D txDeferredPos : register(t0);
//ワールド法線
Texture2D txDeferredNormal : register(t1);
//ディフューズ色
Texture2D txDeferredColor : register(t2);
//ビュー空間での位置
Texture2D txDeferredViewPos : register(t3);
//アンビエントオクルージョン項
Texture2D txDeferredAO : register(t4);

//深度バッファ用
struct ShadowOut {
	float4 pos :SV_POSITION;
};
//MRT用構造体
struct GBufferPS_IN {
	float4 pos : SV_POSITION;
	float4 worldPos : WPOSITION0;
	float4 normal : NORMAL0;
	float4 tangent : NORMAL1;
	float4 binormal : NORMAL2;
	float2 tex : TEXCOORD0;
	float4 viewPos : TEXCOORD1;
};
struct MRTOutput {
	float4 pos : SV_Target0;
	float4 normal : SV_Target1;
	float4 color : SV_Target2;
	float4 viewPos : SV_Target3;
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

inline float4x4 TangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
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
	output.pos = mul(vs_in.pos, view);
	output.pos = mul(vs_in.pos, projection);
	return output;
}
//G-Buffer作成
GBufferPS_IN CreateGBufferVS(VS_IN vs_in)
{
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

	return vs_out;
}
//
//[maxvertexcount(3)]
//void GS(triangle GS_OUT gs_in[3], inout TriangleStream<GS_OUT> stream) {
//	GS_OUT gs_out[3] = (GS_OUT[3])0;
//	//接空間算出 げーむつくろーから。理解は一旦後回し。
//	//5次元から3次元に
//	float3 cp0[3] = (float3[3])0;
//	cp0[0] = float3(gs_in[0].environmentPos.x, gs_in[0].tex.x, gs_in[0].tex.y);
//	cp0[1] = float3(gs_in[0].environmentPos.y, gs_in[0].tex.x, gs_in[0].tex.y);
//	cp0[2] = float3(gs_in[0].environmentPos.z, gs_in[0].tex.x, gs_in[0].tex.y);
//	float3 cp1[3] = (float3[3])0;
//	cp1[0] = float3(gs_in[1].environmentPos.x, gs_in[1].tex.x, gs_in[1].tex.y);
//	cp1[1] = float3(gs_in[1].environmentPos.y, gs_in[1].tex.x, gs_in[1].tex.y);
//	cp1[2] = float3(gs_in[1].environmentPos.z, gs_in[1].tex.x, gs_in[1].tex.y);
//	float3 cp2[3] = (float3[3])0;
//	cp2[0] = float3(gs_in[2].environmentPos.x, gs_in[2].tex.x, gs_in[2].tex.y);
//	cp2[1] = float3(gs_in[2].environmentPos.y, gs_in[2].tex.x, gs_in[2].tex.y);
//	cp2[2] = float3(gs_in[2].environmentPos.z, gs_in[2].tex.x, gs_in[2].tex.y);
//	//平面からＵＶ座標算出
//	float u[3] = (float[3])0;
//	float v[3] = (float[3])0;
//	for (int i = 0; i < 3; i++) {
//		float3 v0 = cp1[i] - cp0[i];
//		float3 v1 = cp2[i] - cp1[i];
//		float3 crossVector = cross(v0, v1);
//		//縮退だけどどうしようもない
//		//if (crossVector.x == 0.0f);
//		u[i] = -crossVector.y / crossVector.x;
//		v[i] = -crossVector.z / crossVector.x;
//	}
//	float4 tangent = normalize(float4(u[0], u[1], u[2], 0.0f));
//	float4 binormal = normalize(float4(v[0], v[1], v[2], 0.0f));
//	float4 normal = float4(normalize(cross(tangent, binormal)), 0.0f);
//	//接空間へ変換するための行列
//	const column_major float4x4 tangentMatrix = InvTangentMatrix(tangent, binormal, normal);
//	float4 tangentLight = normalize(mul(lightDirection, tangentMatrix));
//	for (int i = 0; i < 3; i++) {
//		gs_out[i].pos = gs_in[i].pos;
//		gs_out[i].normal.xyz = normal;
//		gs_out[i].tangent.xyz = tangent;
//		gs_out[i].binormal.xyz = binormal;
//		gs_out[i].tangentLight = tangentLight;
//		gs_out[i].eyeVector = normalize(mul(gs_in[i].eyeVector, tangentMatrix));
//		//環境マップ用のカメラから位置へのベクトルを接空間へ変換
//		gs_out[i].environmentEyeVector = normalize(mul(gs_in[i].environmentEyeVector, tangentMatrix));
//		gs_out[i].tex = gs_in[i].tex;
//		//ストリームに出力
//		stream.Append(gs_out[i]);
//	}
//	stream.RestartStrip();
//}

////深度マップを作成
//float4 CreateShadowMapPS(ShadowOut input) {
//	//αテスト ある程度αがあればシャドウが落ちない
//	clip(txDiffuse.Sample(samLinear, input.tex).a - 0.9f);
//	return input.pos.z / input.pos.w;
//}
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
#ifdef __DEBUG
	//デバッグ表示用
	output.normal.a = 1.0f;
#endif
	output.viewPos = ps_in.viewPos;
	output.viewPos.a = 1.0f;
	return output;
}

//G-Bufferを用いたシェーディング
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

float4 PointLightDeferredPS(PS_IN_TEX ps_in) :SV_Target{
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
	//この先ポイントライト Tiled - Based Renderingにしても良いかも(?)
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