#include "GPUParticleDefine.hlsli"
#include "../Header.hlsli"
#include "../Common/Common.hlsli"

//追加用バッファ
RWByteAddressBuffer appendData : register(u0);
//インデックスバッファ
RWByteAddressBuffer particleIndex : register(u1);
//描画用
RWByteAddressBuffer particleData : register(u2);
//引数用バッファ
RWByteAddressBuffer indirectArgs : register(u3);

Texture2D txDiffuse8 : register(t10);
Texture2D txDiffuse9 : register(t11);
Texture2D txDiffuse10 : register(t12);
Texture2D txDiffuse11 : register(t13);
Texture2D txDiffuse12 : register(t14);
Texture2D txDiffuse13 : register(t15);
Texture2D txDiffuse14 : register(t16);
Texture2D txDiffuse15 : register(t17);

SamplerState samPoint :register(s0);

column_major float4x4 GetIdentityMatrix() {
	static column_major float4x4 identity = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
	return identity;
}

cbuffer GPUParticleInfo : register(b0) {
	int appendCount : packoffset(c0.x);
	float elapsedTime : packoffset(c0.y);
	int compareInterval : packoffset(c0.z);
	int divideLevel : packoffset(c0.w);
	int isBitonicFinal : packoffset(c1.x);
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	AppendParticle
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//描画用バッファにデータを追加
inline void AppendParticleBuffer(uint data_index) {
	uint appendDataIndex = data_index * PARTICLE_DATA_SIZE;
	uint particleDataIndex = asuint(particleIndex.Load(data_index * 4)) * PARTICLE_DATA_SIZE;
	//pos
	particleData.Store3(particleDataIndex + PARTICLE_POS_OFFSET, appendData.Load3(appendDataIndex + PARTICLE_POS_OFFSET));
	//move
	particleData.Store3(particleDataIndex + PARTICLE_MOVE_OFFSET, appendData.Load3(appendDataIndex + PARTICLE_MOVE_OFFSET));
	//power
	particleData.Store3(particleDataIndex + PARTICLE_POWER_OFFSET, appendData.Load3(appendDataIndex + PARTICLE_POWER_OFFSET));
	//textureIndex
	particleData.Store(particleDataIndex + PARTICLE_TEX_INDEX_OFFSET, appendData.Load(appendDataIndex + PARTICLE_TEX_INDEX_OFFSET));
	//uvPos
	particleData.Store2(particleDataIndex + PARTICLE_UV_POS_OFFSET, appendData.Load2(appendDataIndex + PARTICLE_UV_POS_OFFSET));
	//uvSize
	particleData.Store2(particleDataIndex + PARTICLE_UV_SIZE_OFFSET, appendData.Load2(appendDataIndex + PARTICLE_UV_SIZE_OFFSET));
	//aliveTime
	particleData.Store(particleDataIndex + PARTICLE_ALIVE_TIME_OFFSET, appendData.Load(appendDataIndex + PARTICLE_ALIVE_TIME_OFFSET));
	//elapseTime
	particleData.Store(particleDataIndex + PARTICLE_ELAPSED_TIME_OFFSET, appendData.Load(appendDataIndex + PARTICLE_ELAPSED_TIME_OFFSET));
	//fadeInTime
	particleData.Store(particleDataIndex + PARTICLE_FADEIN_TIME_OFFSET, appendData.Load(appendDataIndex + PARTICLE_FADEIN_TIME_OFFSET));
	//fadeOutTime
	particleData.Store(particleDataIndex + PARTICLE_FADEOUT_TIME_OFFSET, appendData.Load(appendDataIndex + PARTICLE_FADEOUT_TIME_OFFSET));
	//startScale
	particleData.Store(particleDataIndex + PARTICLE_START_SCALE_OFFSET, appendData.Load(appendDataIndex + PARTICLE_START_SCALE_OFFSET));
	//endScale
	particleData.Store(particleDataIndex + PARTICLE_END_SCALE_OFFSET, appendData.Load(appendDataIndex + PARTICLE_END_SCALE_OFFSET));
	//startScale
	particleData.Store(particleDataIndex + PARTICLE_START_RAD_OFFSET, appendData.Load(appendDataIndex + PARTICLE_START_RAD_OFFSET));
	//endScale
	particleData.Store(particleDataIndex + PARTICLE_END_RAD_OFFSET, appendData.Load(appendDataIndex + PARTICLE_END_RAD_OFFSET));
	//color
	particleData.Store3(particleDataIndex + PARTICLE_COLOR_OFFSET, appendData.Load3(appendDataIndex + PARTICLE_COLOR_OFFSET));
}
//スレッド数 ＝ 追加したいパーティクル総数 / 1スレッドで追加する数 + 1(繰り上げ)
[numthreads(APPEND_PARTICLE_MAX / THREAD_PER_COUNT, 1, 1)]
void AppendParticle(uint3 thread_id : SV_DispatchThreadID) {
	//1スレッドで複数のパーティクルを処理
	for (int i = 0; i < THREAD_PER_COUNT; i++) {
		//スレッド番号×最大処理数＋ループ処理番号
		uint dataIndex = thread_id.x * THREAD_PER_COUNT + i;
		//追加数を超えるものは除外
		if (dataIndex >= (uint) appendCount)	return;
		//パーティクル最大数制限 
		if (CHECK_PARTICLE_MAX_COUNT(dataIndex))	return;
		//パーティクル追加処理
		AppendParticleBuffer(dataIndex);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	SortParticle
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void BitonicSort(uint thread_id, uint data_index[2]) {
	//対象のデータアドレスを取得
	uint particleAddress[2];
	particleAddress[0] = asuint(particleIndex.Load(data_index[0]));
	particleAddress[1] = asuint(particleIndex.Load(data_index[1]));
	//対象パーティクルの生存時間を取得し(ソート基準)
	bool isAlive[2];
	isAlive[0] = (bool)(0.0f < asfloat((particleData.Load(particleAddress[0] * PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET))));
	isAlive[1] = (bool)(0.0f < asfloat((particleData.Load(particleAddress[1] * PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET))));
	//昇順か否か
	bool isSortModeASC = !(bool)(thread_id / divideLevel % 2);
	//交換するか否か
	bool isTrade = (bool)(isAlive[0] == isSortModeASC);
	//交換を上の結果に基づき行う
	particleIndex.Store(data_index[0], asuint(particleAddress[0 ^ isTrade]));
	particleIndex.Store(data_index[1], asuint(particleAddress[1 ^ isTrade]));
}
inline void ConfigureIndirectArgs(uint thread_id, uint data_index[2]) {
	//------------------------------------------------------------------------------------
	//	配列の中身 [0] 死亡パーティクル->[MAX]生存パーティクル
	//------------------------------------------------------------------------------------
	//自分と上下の要素の死亡数
	uint deadCount = 0;
	//自分と上下の要素番号取得用
	uint aroundIndex;
	//自分よりも一つ前の要素の確認
	//もしもdata_index[0]==0の時は、thread_idが0であることを示し、1つ前の要素-1は存在しないので死亡扱いとする
	if (data_index[0] == 0)deadCount++;
	else {
		//一つ上の要素番号
		aroundIndex = asuint(particleIndex.Load(data_index[0] - 4));
		deadCount += (uint)(0.0f >= asfloat(particleData.Load(aroundIndex*PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET)));
	}
	//自分の確認
	aroundIndex = asuint(particleIndex.Load(data_index[0]));
	deadCount += (uint)(0.0f >= asfloat(particleData.Load(aroundIndex*PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET)));
	//1つ後の要素の確認
	aroundIndex = asuint(particleIndex.Load(data_index[0] + 4));
	deadCount += (uint)(0.0f >= asfloat(particleData.Load(aroundIndex*PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET)));
	//死亡数が1以上で且、生存パーティクルが存在する時生死の境界
	if (deadCount >= 1 && deadCount <= 2) {
		//死亡パーティクル総数 = 自分 - 1 + 3要素の死亡数
		deadCount = (int)thread_id + (int)deadCount;
		//境界を発見したので、引数バッファの構築
		indirectArgs.Store(INDIRECT_ARGS_INDEX_COUNT, asuint(GPU_PARTICLE_MAX - deadCount));
		indirectArgs.Store(INDIRECT_ARGS_INSTANCE_COUNT, asuint(1));
		indirectArgs.Store(INDIRECT_ARGS_START_LOCATION, asuint(deadCount));
		indirectArgs.Store(INDIRECT_ARGS_VERTEX_LOCATION, asuint(0));
		indirectArgs.Store(INDIRECT_ARGS_INSTANCE_LOCATION, asuint(0));
	}
}
[numthreads(LOCAL_THREAD_COUNT, 1, 1)]
void SortParticle(uint3 thread_id : SV_DispatchThreadID) {
	indirectArgs.Store(INDIRECT_ARGS_INDEX_COUNT, asuint(0));
	//2要素を一つのスレッドで行うため、重複するものはスキップ
	bool skip = (bool)(thread_id.x / compareInterval % 2);
	if (skip)	return;
	//対象のインデックスを算出
	uint dataIndex[2];
	dataIndex[0] = thread_id.x * 4;
	dataIndex[1] = (thread_id.x + compareInterval) * 4;
	//ソート実行(このソートの性質上、CPU側で複数回実行されてソートが完了する)
	BitonicSort(thread_id.x, dataIndex);
	//スレッド同期待ち
	GroupMemoryBarrierWithGroupSync();
	//バイトニックソートが最終ステップであるなら
	if (isBitonicFinal)	ConfigureIndirectArgs(thread_id.x, dataIndex);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	UpdateParticle
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void UpdateIndividualParticle(uint data_offset) {
	const uint aliveOffset = data_offset + PARTICLE_ELAPSED_TIME_OFFSET;
	const uint maxAliveOffset = data_offset + PARTICLE_ALIVE_TIME_OFFSET;
	//現在の経過時間取得
	float time = asfloat(particleData.Load(aliveOffset));
	//時間経過
	time -= elapsedTime;
	particleData.Store(aliveOffset, asuint(time));
	if (time < 0)return;
	float maxTime = asfloat(particleData.Load(maxAliveOffset));
	float elapsedTimeSum = maxTime - time;
	const uint posOffset = data_offset + PARTICLE_POS_OFFSET;
	const uint moveOffset = data_offset + PARTICLE_MOVE_OFFSET;
	const uint powerOffset = data_offset + PARTICLE_POWER_OFFSET;
	float3 pos = asfloat(particleData.Load3(posOffset));
	float3 move = asfloat(particleData.Load3(moveOffset));
	float3 power = asfloat(particleData.Load3(powerOffset));
	//移動
	//move = move * elapsedTime + 0.5f * power * (elapsedTime * elapsedTime);
	//上とイコール 展開すると公式通り上になる
	move = ((elapsedTime * power * 0.5f) + move) * elapsedTime;
	pos += move;
	move /= elapsedTime;
	//適用
	particleData.Store3(posOffset, asuint(pos));
	particleData.Store3(moveOffset, asuint(move));
}
[numthreads(LOCAL_THREAD_COUNT, 1, 1)]
void UpdateParticle(uint3 thread_id : SV_DispatchThreadID) {
	for (int i = 0; i < THREAD_PER_COUNT; i++)
	{
		uint dataIndex = thread_id.x * THREAD_PER_COUNT + i;
		if (CHECK_PARTICLE_MAX_COUNT(dataIndex))return;
		dataIndex *= PARTICLE_DATA_SIZE;
		//更新
		UpdateIndividualParticle(dataIndex);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	描画用構造体
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct GS_IN {
	float3 pos:POSITION;
	float3 move:MOVE;
	float3 power:POWER;
	int textureIndex : TEXTUREINDEX;
	float2 uvPos:UVPOS;
	float2 uvSize:UVSIZE;
	float aliveTime : ALIVETIME;
	float elapsedTime : ELAPSEDTIME;
	float fadeInTime : FADEINTIME;
	float fadeOutTime : FADEOUTTIME;
	float startScale : STARTSCALE;
	float endScale : ENDSCALE;
	float startRad : STARTRAD;
	float endRad : ENDRAD;
	float3 color:COLOR;
};
struct GS_OUT {
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
	float4 color : TEXCOORD1;
	int texIndex : TEXCOORD2;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	VertexShader
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//頂点シェーダーはスルー
GS_IN GPUParticleVS(GS_IN vs_in) {
	return vs_in;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	GeometryShader
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GPUParticleGS(point GS_IN gs_in[1], inout TriangleStream<GS_OUT> triangle_stream) {
	//進行率
	float nowRate = 1.0f - gs_in[0].elapsedTime / gs_in[0].aliveTime;
	float scale = lerp(gs_in[0].startScale, gs_in[0].endScale, nowRate);
	if (!IsFrustumRange(gs_in[0].pos, sqrt(scale*scale * 2)))return;
	float angle = lerp(gs_in[0].startRad, gs_in[0].endRad, nowRate);
	float invElapsedTime = gs_in[0].aliveTime - gs_in[0].elapsedTime;
	float alpha = 1.0f;
	if (invElapsedTime <= gs_in[0].fadeInTime)alpha = (float)invElapsedTime / gs_in[0].fadeInTime;
	else if (invElapsedTime >= gs_in[0].fadeOutTime)alpha = 1.0f - (float)(invElapsedTime - gs_in[0].fadeOutTime) / (float)(gs_in[0].aliveTime - gs_in[0].fadeOutTime);
	//ワールド変換行列作成
	matrix wvp = GetIdentityMatrix();
	wvp._11_21 = float2(cos(angle), sin(angle));
	wvp._12_22 = float2(-sin(angle), cos(angle));
	//ビルボード行列からwvp変換行列作成
	wvp = mul(transpose(billboardMat), wvp);
	wvp._14_24_34 = gs_in[0].pos;
	matrix vp = transpose(view);
	vp = mul(transpose(projection), vp);
	wvp = mul(vp, wvp);
	GS_OUT gs_out[4] = (GS_OUT[4])0;
	//頂点作成
	gs_out[0].pos = mul(wvp, float4(0.5*scale, 0.5*scale, 0.0f, 1.0f));
	gs_out[1].pos = mul(wvp, float4(-0.5*scale, 0.5*scale, 0.0f, 1.0f));
	gs_out[2].pos = mul(wvp, float4(0.5*scale, -0.5*scale, 0.0f, 1.0f));
	gs_out[3].pos = mul(wvp, float4(-0.5*scale, -0.5*scale, 0.0f, 1.0f));
	//uv設定
	gs_out[0].tex = gs_in[0].uvPos;
	gs_out[1].tex = gs_in[0].uvPos + float2(gs_in[0].uvSize.x, 0.0f);
	gs_out[2].tex = gs_in[0].uvPos + float2(0.0f, gs_in[0].uvSize.y);
	gs_out[3].tex = gs_in[0].uvPos + gs_in[0].uvSize;
	//アルファ値設定
	gs_out[0].color = float4(gs_in[0].color.rgb, alpha);
	gs_out[1].color = float4(gs_in[0].color.rgb, alpha);
	gs_out[2].color = float4(gs_in[0].color.rgb, alpha);
	gs_out[3].color = float4(gs_in[0].color.rgb, alpha);
	//テクスチャ設定
	gs_out[0].texIndex = gs_in[0].textureIndex;
	gs_out[1].texIndex = gs_in[0].textureIndex;
	gs_out[2].texIndex = gs_in[0].textureIndex;
	gs_out[3].texIndex = gs_in[0].textureIndex;
	//ストリームに出力
	triangle_stream.Append(gs_out[0]);
	triangle_stream.Append(gs_out[1]);
	triangle_stream.Append(gs_out[2]);
	triangle_stream.Append(gs_out[3]);
	triangle_stream.RestartStrip();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	PixelShader
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float4 GPUParticlePS(GS_OUT gs_out) : SV_Target{
	float4 color;
	switch (gs_out.texIndex) {
	case 0:	color = txDiffuse8.Sample(samPoint, gs_out.tex);  break;
	case 1:	color = txDiffuse9.Sample(samPoint, gs_out.tex);  break;
	case 2:	color = txDiffuse10.Sample(samPoint, gs_out.tex); break;
	case 3:	color = txDiffuse11.Sample(samPoint, gs_out.tex); break;
	case 4:	color = txDiffuse12.Sample(samPoint, gs_out.tex); break;
	case 5:	color = txDiffuse13.Sample(samPoint, gs_out.tex); break;
	case 6:	color = txDiffuse14.Sample(samPoint, gs_out.tex); break;
	case 7:	color = txDiffuse15.Sample(samPoint, gs_out.tex); break;
	default:	color = float4(1.0f,1.0f,1.0f,1.0f);							 break;
	}
	return color * gs_out.color;
}
