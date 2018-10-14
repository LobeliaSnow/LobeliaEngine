#pragma once
//デュアルパラボロイドマップを用いた環境マップを使用
//#define __PARABOLOID__
#define LIGHT_SUM 128
//#define SIMPLE_SHADER
#define FULL_EFFECT
//SSAO用定義
#define SSAO_BLOCK_SIZE 32
#define SSAO_RECT_RANGE 8
//ガウスフィルタ用
//周囲何ピクセルをフェッチするか
#define GAUSSIAN_BLOCK 32
//テスト用
#define USE_SSAO
//#define SSAO_PS
#define SSAO_CS
#define GAUSSIAN_PS
//#define GAUSSIAN_CS