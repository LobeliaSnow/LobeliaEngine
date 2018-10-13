#include "../Define.h"

//参照
//https://sites.google.com/site/monshonosuana/directxno-hanashi-1/directx-109

//定数バッファ
cbuffer SSAO :register(b7) {
	//チェックする深度の範囲
	float offsetPerPixel : packoffset(c0.x);
	int useAO : packoffset(c0.y);
}
cbuffer GaussianFilter : register(b9) {
	float  weight0 : packoffset(c0.x);    // 重み
	float  weight1 : packoffset(c0.y);    // 重み
	float  weight2 : packoffset(c0.z);    // 重み
	float  weight3 : packoffset(c0.w);    // 重み
	float  weight4 : packoffset(c1.x);    // 重み
	float  weight5 : packoffset(c1.y);    // 重み
	float  weight6 : packoffset(c1.z);    // 重み
};
// static const float offsetPerPixel = 20.0f;
//入力用
Texture2D inputTex :register(t0);
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