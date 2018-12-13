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
#define UVTRANS_MATRIX float4x4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f)

//#include "../Header.hlsli"
//
////ライト空間へ変換
//void ConverteLVP(out float4 lvp_pos, out float4 light_uv, in float4 world_pos, in Texture2DArray shadow_map, in Texture2D data_tex, int index) {
//	column_major float4x4 lvp = (float4x4)0.0f;
//	//LVP復元
//	lvp[0][0] = data_tex.Load(int3(1, index, 0));	lvp[0][1] = data_tex.Load(int3(2, index, 0));	lvp[0][2] = data_tex.Load(int3(3, index, 0));	lvp[0][3] = data_tex.Load(int3(4, index, 0));
//	lvp[1][0] = data_tex.Load(int3(5, index, 0));	lvp[1][1] = data_tex.Load(int3(6, index, 0));	lvp[1][2] = data_tex.Load(int3(7, index, 0));	lvp[1][3] = data_tex.Load(int3(8, index, 0));
//	lvp[2][0] = data_tex.Load(int3(9, index, 0));	lvp[2][1] = data_tex.Load(int3(10, index, 0));	lvp[2][2] = data_tex.Load(int3(11, index, 0));	lvp[2][3] = data_tex.Load(int3(12, index, 0));
//	lvp[3][0] = data_tex.Load(int3(13, index, 0));	lvp[3][1] = data_tex.Load(int3(14, index, 0));	lvp[3][2] = data_tex.Load(int3(15, index, 0));	lvp[3][3] = data_tex.Load(int3(16, index, 0));
//	lvp_pos = mul(world_pos, lvp);
//	light_uv = mul(lvp_pos, UVTRANS_MATRIX);
//}
////vp_posはNDCに直さないで送ってください
//float ApplyCascadeVarianceShadow(in int split_count, in float4 vp_pos, in Texture2DArray shadow_map, in Texture2D data_tex) {
//	//長さ
//	float lightSpaceLength = vp_pos.w;
//	int index = 0;
//	for (int i = 0; i < split_count; i++) {
//		float splitPos = data_tex.Load(int3(0, i, 0));
//		if (lightSpaceLength < splitPos) {
//			index = i;
//			break;
//		}
//	}
//	float4 lvpPos = (float4)0.0f;
//	float4 lightUV = (float4)0.0f;
//	ConverteLVP(lvpPos, lightUV)
//	shadow_map.Sample(samLinear, float3(light_uv, index)).rg;
//}