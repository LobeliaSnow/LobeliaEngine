#include "../Header.hlsli"
#include "../2D/Header2D.hlsli"
#include "CodecGBuffer.hlsli"

Texture2D<uint4> txData0 : register(t0);
Texture2D<uint4> txData1 : register(t1);

//情報をデコードして、個別描画するためのシェーダー
float4 DecodeSDRColorPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeColor = LoadUintTexture(txData0, ps_in.tex).r;
	return DecodeSDRColor(encodeColor);
}
float4 DecodeDepthPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeDepth = LoadUintTexture(txData0, ps_in.tex).b;
	float depth = DecodeDepth(encodeDepth);
	return float4((float3)depth, 1.0f);
}
float4 DecodeWorldPosPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeDepth = LoadUintTexture(txData0, ps_in.tex).b;
	float depth = DecodeDepth(encodeDepth);
	return DecodeDepthToWorldPos(depth, ps_in.tex, inverseViewProjection);
}
float4 DecodeNormalVectorPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeNormal = LoadUintTexture(txData0, ps_in.tex).g;
	return float4(DecodeNormalVector(encodeNormal).xyz,1.0f);
}
float4 DecodeLightingIntensityPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeLightingPower = LoadUintTexture(txData0, ps_in.tex).a;
	return float4((float3)DecodeLightingIntensity(encodeLightingPower),1.0f);
}
float4 DecodeSpecularIntensityPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeLightingPower = LoadUintTexture(txData0, ps_in.tex).a;
	return float4((float3)DecodeSpecularIntensity(encodeLightingPower),1.0f);
}
float4 DecodeEmissionPS(PS_IN_TEX ps_in) : SV_Target{
	uint2 encodeData = LoadUintTexture(txData0, ps_in.tex).rg;
	float4 emission = DecodeSDRColor(encodeData.r);
	emission.rgb *= f16tof32(encodeData.g);
	return emission;
}
float4 DecodeEmissionIntensityPS(PS_IN_TEX ps_in) : SV_Target{
	uint encodeEmissionPower = LoadUintTexture(txData0, ps_in.tex).g;
	return float4((float3)f16tof32(encodeEmissionPower), 1.0f);
}