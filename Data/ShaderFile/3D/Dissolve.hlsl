#include "../Header.hlsli"
#include "Header3D.hlsli"

//---------------------------------------------------------------------------------------------------------------
//関数
//---------------------------------------------------------------------------------------------------------------
//テクスチャサンプリング
inline float4 GetDiffuseMap(float2 uv)
{
	return txDiffuse.Sample(samLinear, uv);
}
inline float4 LightingBias(float3 normalized_normal, float3 normalized_light_direction)
{
	float4 ret = saturate(dot(normalized_normal, normalized_light_direction.xyz));
	ret.a = 1.0f;
	return ret;
}
inline float4 SpecularCalc(float2 uv, float3 normalized_normal, float3 normalized_eye_vector, float3 normalized_light_direction, float3 light_bias)
{
	//フォン
	float3 reflect = normalize(light_bias.xyz * normalized_normal - normalized_light_direction.xyz);
	//スペキュラマップ考慮のスペキュラ値計算
	//現状直値だが、powの第二引数で強さを変えることができる
	return pow(dot(reflect, normalized_eye_vector), 4);
}

cbuffer DissolveInfo : register(b7) {
	//閾値
	float threshold : packoffset(c0.x);
};
//ディゾルブ用
Texture2D txDissolveMap: register(t3);

//ピクセルシェーダーの差し替えだけで良い
float4 PS(PS_IN ps_in) : SV_Target {
	//ディゾルブ用のテクスチャの値をフェッチ
	float dissolve = txDissolveMap.Sample(samLinear, ps_in.tex);
	//閾値を下回っていたら描画しない
	if (dissolve <= threshold)return float4(0.0f,0.0f,0.0f,0.0f);
	//フォン
	float3 normal = normalize(ps_in.normal);
	float3 viewDir = normalize(ps_in.eyeVector);
	float4 normalLight = LightingBias(normal, lightDirection.xyz);
	//スペキュラマップ考慮のスペキュラ値計算
	float4 specular = SpecularCalc(ps_in.tex, normal, viewDir, lightDirection.xyz, normalLight.xyz);
	float4 diffuseColor = GetDiffuseMap(ps_in.tex) * texColor * ambientColor;
	normalLight.a = 1.0f;
	
	return diffuseColor * normalLight + specular;
}