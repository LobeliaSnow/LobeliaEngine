#pragma once
//デュアルパラボロイドマップを用いた環境マップを使用
//#define __PARABOLOID__
#define LIGHT_SUM 128
//#define SIMPLE_SHADER
#define POINT_LIGHT
//SSAO用定義
#define USE_SSAO
#define SSAO_BLOCK_SIZE 32
#define SSAO_RECT_RANGE 8
//ガウスフィルタ用
//周囲何ピクセルをフェッチするか
#define GAUSSIAN_BLOCK 32