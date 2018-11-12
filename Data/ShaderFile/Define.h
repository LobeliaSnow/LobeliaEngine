#pragma once
//デュアルパラボロイドマップを用いた環境マップを使用
//#define __PARABOLOID__
#define LIGHT_SUM 256
//#define SIMPLE_SHADER
#define FULL_EFFECT
//SSAO用定義
#define SSAO_BLOCK_SIZE 32
#define SSAO_RECT_RANGE 8
//ガウスフィルタ用
#define GAUSSIAN_BLOCK 32
//テスト用
#define USE_SSAO
//#define SSAO_PS
#define SSAO_CS
#define GAUSSIAN_PS
//#define GAUSSIAN_CS
#define CASCADE
#define USE_DOF
//クオリティ設定、影とSSAOの解像度にかけられる倍率
//SSAOには倍率1.0f以上はありません
//高
//#define QUALITY 2.0f
//中
//#define QUALITY 1.0f
//低
#define QUALITY 0.5f
#define DOF_QUALITY 1.0f
//#define DOF_QUALITY QUALITY

//#define USE_CHARACTER
#define CPU_RAYCASTER
//#define GPU_RAYCASTER

#define USE_MOTION_BLUR

#define USE_HDR

//マテリアル設定
#define MAT_LAMBERT 0
#define MAT_PHONG 1
#define MAT_COLOR 2