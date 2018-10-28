#include "../Define.h"
#include "Header2D.hlsli"

//参照
//https://sites.google.com/site/monshonosuana/directxno-hanashi-1/directx-109

//定数バッファ
cbuffer SSAO :register(b7) {
	//チェックする深度の範囲
	float offsetPerPixel : packoffset(c0.x);
	//AO使うか否か
	int useAO : packoffset(c0.y);
	//PS用
	float offsetPerPixelX : packoffset(c0.z);
	float offsetPerPixelY : packoffset(c0.w);
}
cbuffer GaussianFilter : register(b9) {
	float  weight0 : packoffset(c0.x);
	float  weight1 : packoffset(c0.y);
	float  weight2 : packoffset(c0.z);
	float  weight3 : packoffset(c0.w);
	float  weight4 : packoffset(c1.x);
	float  weight5 : packoffset(c1.y);
	float  weight6 : packoffset(c1.z);
	//PS専用
	float  width : packoffset(c1.w);
	float  height : packoffset(c2.x);
};
cbuffer DoF :register(b12) {
	float focusRange : packoffset(c0.x);
}
// static const float offsetPerPixel = 20.0f;
//入力用
Texture2D inputTex :register(t0);
//被写界深度用
Texture2D inputDepth :register(t1);
Texture2D inputBokeh0 :register(t2);
Texture2D inputBokeh1 :register(t3);
//CS実装
//出力用
RWTexture2D<float4> outputTex :register(u0);
//共有メモリ
//SSAO用
groupshared float cacheDepth[SSAO_BLOCK_SIZE * SSAO_BLOCK_SIZE * 4];
//GaussiannFilter用 適当の為、もっと少なくて済むと思う。
groupshared float4 cacheDiffuse[GAUSSIAN_BLOCK * GAUSSIAN_BLOCK * 2];
//共通
//ピクセル位置を取得
uint2 GetInputPixelIndex(int2 base_pos, uint2 pos) {
	return base_pos + pos;
}
//そのスレッドグループでの基準となる位置算出(真ん中)
int2 CalcBasePos(float2 group_id, int block_size) {
	return group_id.xy * block_size - (block_size / 2);
}
//周囲四ピクセルずつフェッチする場合用
//取るべきピクセルインデックスが帰ってくる
int2 CalcIndex(int2 group_thread_id, int block_size) {
	//X上側/Y下側
	return int2(group_thread_id.y * (block_size * 2) + group_thread_id.x, (group_thread_id.y + block_size) * (block_size * 2) + group_thread_id.x);
}
//SSAO用
float GetCachePixel(int2 pos) {
	return cacheDepth[pos.y * (SSAO_BLOCK_SIZE * 2) + pos.x];
}
[numthreads(SSAO_BLOCK_SIZE, SSAO_BLOCK_SIZE, 1)]
void SSAOCS(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID, uint3 group_id : SV_GroupID) {
	//スレッドグループの存在位置を全体スレッドの中から算出
	int2 basePos = CalcBasePos(group_id, SSAO_BLOCK_SIZE);
	//上下左右SSAO_BLOCK_SIZE分離れた位置を算出
	int2 texelIndex = CalcIndex(group_thread_id, SSAO_BLOCK_SIZE);
	//キャッシュをためる
	cacheDepth[texelIndex.x] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy)].z;
	cacheDepth[texelIndex.x + SSAO_BLOCK_SIZE] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy + uint2(SSAO_BLOCK_SIZE, 0))].z;
	cacheDepth[texelIndex.y] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy + uint2(0, SSAO_BLOCK_SIZE))].z;
	cacheDepth[texelIndex.y + SSAO_BLOCK_SIZE] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy + uint2(SSAO_BLOCK_SIZE, SSAO_BLOCK_SIZE))].z;
	//スレッドの同期待ち
	//これにより、共有メモリがいつ参照されても問題がなくなる
	GroupMemoryBarrierWithGroupSync();
	//デバッグ用 半分だけ描画
	// if(thread_id.x >= 640)return;
	//自身のピクセルをキャッシュにより取得 影響範囲(自分のスレッドグループ内部の矩形)
	int2 pos = group_thread_id.xy + (SSAO_BLOCK_SIZE / 2);
	float own = cacheDepth[pos.y * (SSAO_BLOCK_SIZE * 2) + pos.x];
	pos -= SSAO_RECT_RANGE;
	float occ = 0.0f;
	float pixel = 0.0f;
	for (int y = 0; y <= SSAO_RECT_RANGE * 2; y++) {
		int index = (pos.y + y) * (SSAO_BLOCK_SIZE * 2) + pos.x;
		for (int x = 0; x <= SSAO_RECT_RANGE * 2; x++) {
			float check = cacheDepth[index + x];
			float depth = own - check;
			if (depth <= offsetPerPixel) {
				//チェックの対象
				occ += max(-1.0f, (depth / offsetPerPixel));
				pixel += 1.0f;
			}
		}
	}
	float ret = 1.0f - (occ / pixel);
	outputTex[uint2(thread_id.x, thread_id.y)] = (float4)ret;
}
[numthreads(GAUSSIAN_BLOCK, GAUSSIAN_BLOCK, 1)]
void GaussianFilterCSX(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID, uint3 group_id : SV_GroupID) {
	//キャッシュをためる
	//yを一次元に落とし込んだ時のIndex
	int index = group_thread_id.y * GAUSSIAN_BLOCK * 2;
	//左側フェッチ
	cacheDiffuse[index + group_thread_id.x] = inputTex[thread_id.xy - int2(GAUSSIAN_BLOCK * 0.5f, 0)];
	//右側フェッチ
	cacheDiffuse[index + group_thread_id.x + GAUSSIAN_BLOCK] = inputTex[thread_id.xy + int2(GAUSSIAN_BLOCK * 0.5f, 0)];
	//スレッドの同期待ち
	//これにより、共有メモリがいつ参照されても問題がなくなる
	GroupMemoryBarrierWithGroupSync();
	//ガウスブラー開始
	float4 diffuse = (float4)0;
	//一応+-(GAUSSIAN_BLOCK*0.5f)分のメモリ分フェッチしているからパラメーター追加すれば範囲もいじれる
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f - 4] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f + 1]) * weight0;
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f - 2] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f + 2]) * weight1;
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f - 1] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f + 3]) * weight2;
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f]) * weight3;
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f + 1] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f - 3]) * weight4;
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f + 2] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f - 2]) * weight5;
	diffuse += (cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f + 3] + cacheDiffuse[group_thread_id.x + index + GAUSSIAN_BLOCK * 0.5f - 1]) * weight6;
	outputTex[thread_id.xy] = diffuse;
}
[numthreads(GAUSSIAN_BLOCK, GAUSSIAN_BLOCK, 1)]
void GaussianFilterCSY(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID, uint3 group_id : SV_GroupID) {
	//キャッシュをためる
	//xを一次元に落とし込んだ時のIndex
	int index = group_thread_id.x * GAUSSIAN_BLOCK * 2;
	//上側フェッチ
	cacheDiffuse[index + group_thread_id.y] = inputTex[thread_id.xy - int2(0, GAUSSIAN_BLOCK * 0.5f)];
	//下側フェッチ
	cacheDiffuse[index + group_thread_id.y + GAUSSIAN_BLOCK] = inputTex[thread_id.xy + int2(0, GAUSSIAN_BLOCK * 0.5f)];
	//スレッドの同期待ち
	//これにより、共有メモリがいつ参照されても問題がなくなる
	GroupMemoryBarrierWithGroupSync();
	//ガウスブラー開始
	float4 diffuse = (float4)0;
	//一応+-(GAUSSIAN_BLOCK*0.5f)分のメモリ分フェッチしているからパラメーター追加すれば範囲もいじれる
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f - 4] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f + 1]) * weight0;
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f - 2] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f + 2]) * weight1;
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f - 1] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f + 3]) * weight2;
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f]) * weight3;
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f + 1] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f - 3]) * weight4;
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f + 2] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f - 2]) * weight5;
	diffuse += (cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f + 3] + cacheDiffuse[group_thread_id.y + index + GAUSSIAN_BLOCK * 0.5f - 1]) * weight6;
	outputTex[thread_id.xy] = diffuse;
}
//ただのモザイク
//[numthreads(GAUSSIANN_SCALE, GAUSSIANN_SCALE, 1)]
//void GaussianFilterCS(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID, uint3 group_id : SV_GroupID) {
//	//スレッドグループの存在位置を全体スレッドの中から算出
//	int2 basePos = CalcBasePos(group_id, GAUSSIANN_SCALE);
//	//上下左右SSAO_BLOCK_SIZE分離れた位置を算出
//	int2 texelIndex = CalcIndex(group_thread_id, GAUSSIANN_SCALE);
//	float2 bias = picSize / outSize;
//	//キャッシュをためる
//	cacheDiffuse[texelIndex.x] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy) * bias];
//	cacheDiffuse[texelIndex.x + GAUSSIANN_SCALE] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy + uint2(GAUSSIANN_SCALE, 0)) * bias];
//	cacheDiffuse[texelIndex.y] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy + uint2(0, GAUSSIANN_SCALE)) * bias];
//	cacheDiffuse[texelIndex.y + GAUSSIANN_SCALE] = inputTex[GetInputPixelIndex(basePos, group_thread_id.xy + uint2(GAUSSIANN_SCALE, GAUSSIANN_SCALE)) * bias];
//	//スレッドの同期待ち
//	//これにより、共有メモリがいつ参照されても問題がなくなる
//	GroupMemoryBarrierWithGroupSync();
//	//自身のピクセルをキャッシュにより取得 影響範囲(自分のスレッドグループ内部の矩形)
//	int2 pos = group_thread_id.xy + (GAUSSIANN_SCALE / 2);
//	pos -= GAUSSIANN_SCALE;
//	float4 color = (float4)0;
//	int pixel = 0;
//	for (int y = 0; y <= GAUSSIANN_SCALE * 2; y++) {
//		int index = (pos.y + y) * (GAUSSIANN_SCALE * 2) + pos.x;
//		for (int x = 0; x <= GAUSSIANN_SCALE * 2; x++) {
//			float4 diffuse = cacheDiffuse[index + x];
//			if (diffuse.a < 0.01f)continue;
//			color += cacheDiffuse[index + x];
//			pixel++;
//		}
//	}
//	outputTex[uint2(thread_id.x, thread_id.y)] = (float4)color / pixel;
//}

//PS実装
SamplerState samLinear : register(s0);

float4 SSAOPS(PS_IN_TEX ps_in) : SV_Target{
	float own = inputTex.Sample(samLinear,ps_in.tex).z;
	float occ = 0.0f;
	float pixel = 0.0f;
	for (int x = -SSAO_RECT_RANGE; x <= SSAO_RECT_RANGE; x++) {
		for (int y = -SSAO_RECT_RANGE; y <= SSAO_RECT_RANGE; y++) {
			float check = inputTex.Sample(samLinear, ps_in.tex + float2(offsetPerPixelX, offsetPerPixelY) * float2(x,y)).z;
			float depth = own - check;
			if (depth <= offsetPerPixel) {
				occ += max(-1.0f, depth / offsetPerPixel);
				pixel += 1.0f;
			}
		}
	}
	float4 ret = 1.0f - occ / pixel;
	ret.a = 1.0f;
	return ret;
}

struct GAUSSIAN_VS_OUT {
	float4 pos    : SV_POSITION;
	float2 tex0 : TEXCOORD0;   // テクセル
	float2 tex1 : TEXCOORD1;   // テクセル
	float2 tex2 : TEXCOORD2;   // テクセル
	float2 tex3 : TEXCOORD3;   // テクセル
	float2 tex4 : TEXCOORD4;   // テクセル
	float2 tex5 : TEXCOORD5;   // テクセル
	float2 tex6 : TEXCOORD6;   // テクセル
	float2 tex7 : TEXCOORD7;   // テクセル
};
GAUSSIAN_VS_OUT GaussianFilterVSX(VS_IN_TEX vs_in) {
	GAUSSIAN_VS_OUT output = (GAUSSIAN_VS_OUT)0;
	output.pos = vs_in.pos;
	output.tex0 = vs_in.tex + float2(-3.0f / width, 0.0f);
	output.tex1 = vs_in.tex + float2(-2.0f / width, 0.0f);
	output.tex2 = vs_in.tex + float2(-1.0f / width, 0.0f);
	output.tex3 = vs_in.tex + float2(0.0f, 0.0f);
	output.tex4 = vs_in.tex + float2(1.0f / width, 0.0f);
	output.tex5 = vs_in.tex + float2(2.0f / width, 0.0f);
	output.tex6 = vs_in.tex + float2(3.0f / width, 0.0f);
	return output;
}
GAUSSIAN_VS_OUT GaussianFilterVSY(VS_IN_TEX vs_in) {
	GAUSSIAN_VS_OUT output = (GAUSSIAN_VS_OUT)0;
	output.pos = vs_in.pos;
	output.tex0 = vs_in.tex + float2(0.0f, -3.0f / height);
	output.tex1 = vs_in.tex + float2(0.0f, -2.0f / height);
	output.tex2 = vs_in.tex + float2(0.0f, -1.0f / height);
	output.tex3 = vs_in.tex + float2(0.0f, 0.0f);
	output.tex4 = vs_in.tex + float2(0.0f, 1.0f / height);
	output.tex5 = vs_in.tex + float2(0.0f, 2.0f / height);
	output.tex6 = vs_in.tex + float2(0.0f, 3.0f / height);
	return output;

}
//共通
float4 GaussianFilterPS(GAUSSIAN_VS_OUT ps_in) :SV_Target{
	float4 ret = (float4)0;
	ret += (inputTex.Sample(samLinear, ps_in.tex0) + inputTex.Sample(samLinear, ps_in.tex6)) * weight0;
	ret += (inputTex.Sample(samLinear, ps_in.tex1) + inputTex.Sample(samLinear, ps_in.tex5)) * weight1;
	ret += (inputTex.Sample(samLinear, ps_in.tex2) + inputTex.Sample(samLinear, ps_in.tex4)) * weight2;
	ret += (inputTex.Sample(samLinear, ps_in.tex3) + inputTex.Sample(samLinear, ps_in.tex3)) * weight3;
	ret += (inputTex.Sample(samLinear, ps_in.tex4) + inputTex.Sample(samLinear, ps_in.tex2)) * weight4;
	ret += (inputTex.Sample(samLinear, ps_in.tex5) + inputTex.Sample(samLinear, ps_in.tex1)) * weight5;
	ret += (inputTex.Sample(samLinear, ps_in.tex6) + inputTex.Sample(samLinear, ps_in.tex0)) * weight6;
	return ret;
}

//Gaussian DoF
//参考
//http://dxlib.o.oo7.jp/program/dxprogram_DepthOfField.html
//http://api.unrealengine.com/JPN/Engine/Rendering/PostProcessEffects/DepthOfField/
//https://t-pot.com/program/32_dof/dof.html
//現状細い部分が怪しい動きをする
float4 GaussianDoFPS(PS_IN_TEX ps_in) :SV_Target{
	//真ん中をピントの中心とする
	float centerDepth = inputDepth.Sample(samLinear, float2(0.5f,0.5f)).z;
	//ピント幅算出
	float focusBegin = centerDepth - focusRange;
	float focusEnd = centerDepth + focusRange;
	//下限を合わせる
	float depth = max(0.0f, inputDepth.Sample(samLinear, ps_in.tex).z - focusBegin);
	//中央値算出
	float center = (focusEnd - focusBegin)*0.5f;
	//この値は中心が0で離れるほど1に近づく
	float fade = 0.0f;
	if (depth <= center)fade = depth / center;
	else fade = 1.0f - (depth - center) / center;
	fade = saturate(1.0f - fade);
	float4 color0; float4 color1; float blendRatio;
	if (fade < 0.5f) {
		//ボケなし
		color0 = inputTex.Sample(samLinear, ps_in.tex);
		//弱ボケ
		color1 = inputBokeh0.Sample(samLinear, ps_in.tex);
		//0.0f~1.0fの比率を算出
		blendRatio = fade / 0.5f;
	}
	else {
		//弱ボケ
		color0 = inputBokeh0.Sample(samLinear, ps_in.tex);
		//強ボケ
		color1 = inputBokeh1.Sample(samLinear, ps_in.tex);
		//0.0f~1.0fの比率を算出
		//-0.5fの理由はこの値が0.5f以上だから
		blendRatio = (fade - 0.5f) / 0.5f;
	}
	float4 color = lerp(color0, color1, blendRatio);
	return color;
}