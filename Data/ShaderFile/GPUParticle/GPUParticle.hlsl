#include "GPUParticleDefine.hlsli"
#include "../Common/Common.hlsli"

//�ǉ��p�o�b�t�@
RWByteAddressBuffer appendData : register(u0);
//�C���f�b�N�X�o�b�t�@
RWByteAddressBuffer particleIndex : register(u1);
//�`��p
RWByteAddressBuffer particleData : register(u2);
//�����p�o�b�t�@
RWByteAddressBuffer indirectArgs : register(u3);

Texture2D txDiffuse0 : register(t10);
Texture2D txDiffuse1 : register(t11);
Texture2D txDiffuse2 : register(t12);
Texture2D txDiffuse3 : register(t13);
Texture2D txDiffuse4 : register(t14);
Texture2D txDiffuse5 : register(t15);
Texture2D txDiffuse6 : register(t16);
Texture2D txDiffuse7 : register(t17);

SamplerState samPoint:register(s0);
//{
//	Filter = MIN_MAG_MIP_POINT;
//    AddressU = Clamp;
//    AddressV = Clamp;
//};

cbuffer GPUParticleInfo : register(b0)
{
	int appendCount : packoffset(c0.x);
	float elapsedTime : packoffset(c0.y);
	int compareInterval : packoffset(c0.z);
	int divideLevel : packoffset(c0.w);
	int isBitonicFinal : packoffset(c1.x);
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	�ėp�֐�
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
column_major float4x4 GetIdentityMatrix()
{
	static column_major float4x4 identity = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
	return identity;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	AppendParticle
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//�`��p�o�b�t�@�Ƀf�[�^��ǉ�
inline void AppendParticleBuffer(uint data_index)
{
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

//�X���b�h�� �� �ǉ��������p�[�e�B�N������ / 1�X���b�h�Œǉ����鐔 + 1(�J��グ)
[numthreads(APPEND_PARTICLE_MAX / THREAD_PER_COUNT, 1, 1)]
void AppendParticle(uint3 thread_id : SV_DispatchThreadID)
{
	//1�X���b�h�ŕ����̃p�[�e�B�N��������
	for (int i = 0; i < THREAD_PER_COUNT; i++)
	{
		//�X���b�h�ԍ��~�ő又�����{���[�v�����ԍ�
		uint dataIndex = thread_id.x * THREAD_PER_COUNT + i;
		//�ǉ����𒴂�����̂͏��O
		if (dataIndex >= (uint) appendCount)	return;
		//�p�[�e�B�N���ő吔���� 
		if (CHECK_PARTICLE_MAX_COUNT(dataIndex))	return;
		//�p�[�e�B�N���ǉ�����
		AppendParticleBuffer(dataIndex);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	SortParticle
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void BitonicSort(uint thread_id, uint data_index[2])
{
	//�Ώۂ̃f�[�^�A�h���X���擾
	uint particleAddress[2];
	particleAddress[0] = asuint(particleIndex.Load(data_index[0]));
	particleAddress[1] = asuint(particleIndex.Load(data_index[1]));
	//�Ώۃp�[�e�B�N���̐������Ԃ��擾��(�\�[�g�)
	bool isAlive[2];
	isAlive[0] = (bool)(0.0f < asfloat((particleData.Load(particleAddress[0] * PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET))));
	isAlive[1] = (bool)(0.0f < asfloat((particleData.Load(particleAddress[1] * PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET))));
	//�������ۂ�
	bool isSortModeASC = !(bool)(thread_id / divideLevel % 2);
	//�������邩�ۂ�
	bool isTrade = (bool)(isAlive[0] == isSortModeASC);
	//��������̌��ʂɊ�Â��s��
	particleIndex.Store(data_index[0], asuint(particleAddress[0 ^ isTrade]));
	particleIndex.Store(data_index[1], asuint(particleAddress[1 ^ isTrade]));
}
inline void ConfigureIndirectArgs(uint thread_id, uint data_index[2])
{
	//------------------------------------------------------------------------------------
	//	�z��̒��g [0] ���S�p�[�e�B�N��->[MAX]�����p�[�e�B�N��
	//------------------------------------------------------------------------------------
	//�����Ə㉺�̗v�f�̎��S��
	uint deadCount = 0;
	//�����Ə㉺�̗v�f�ԍ��擾�p
	uint aroundIndex;
	//����������O�̗v�f�̊m�F
	//������data_index[0]==0�̎��́Athread_id��0�ł��邱�Ƃ������A1�O�̗v�f-1�͑��݂��Ȃ��̂Ŏ��S�����Ƃ���
	if (data_index[0] == 0)deadCount++;
	else {
		//���̗v�f�ԍ�
		aroundIndex = asuint(particleIndex.Load(data_index[0] - 4));
		deadCount += (uint)(0.0f >= asfloat(particleData.Load(aroundIndex*PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET)));
	}
	//�����̊m�F
	aroundIndex = asuint(particleIndex.Load(data_index[0]));
	deadCount += (uint)(0.0f >= asfloat(particleData.Load(aroundIndex*PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET)));
	//1��̗v�f�̊m�F
	aroundIndex = asuint(particleIndex.Load(data_index[0] + 4));
	deadCount += (uint)(0.0f >= asfloat(particleData.Load(aroundIndex*PARTICLE_DATA_SIZE + PARTICLE_ELAPSED_TIME_OFFSET)));
	//���S����1�ȏ�Ŋ��A�����p�[�e�B�N�������݂��鎞�����̋��E
	if (deadCount >= 1 && deadCount <= 2) {
		//���S�p�[�e�B�N������ = ���� - 1 + 3�v�f�̎��S��
		deadCount = (int)thread_id - 1 + (int)deadCount;
		//���E�𔭌������̂ŁA�����o�b�t�@�̍\�z
		indirectArgs.Store(INDIRECT_ARGS_INDEX_COUNT, asuint(GPU_PARTICLE_MAX - deadCount));
		indirectArgs.Store(INDIRECT_ARGS_INSTANCE_COUNT, asuint(1));
		indirectArgs.Store(INDIRECT_ARGS_START_LOCATION, asuint(deadCount));
		indirectArgs.Store(INDIRECT_ARGS_VERTEX_LOCATION, asuint(0));
		indirectArgs.Store(INDIRECT_ARGS_INSTANCE_LOCATION, asuint(0));
	}
}
[numthreads(LOCAL_THREAD_COUNT, 1, 1)]
void SortParticle(uint3 thread_id : SV_DispatchThreadID)
{
	indirectArgs.Store(INDIRECT_ARGS_INDEX_COUNT, asuint(0));
	//2�v�f����̃X���b�h�ōs�����߁A�d��������̂̓X�L�b�v
	bool skip = (bool)(thread_id.x / compareInterval % 2);
	if (skip)	return;
	//�Ώۂ̃C���f�b�N�X���Z�o
	uint dataIndex[2];
	dataIndex[0] = thread_id.x * 4;
	dataIndex[1] = (thread_id.x + compareInterval) * 4;
	//�\�[�g���s(���̃\�[�g�̐�����ACPU���ŕ�������s����ă\�[�g����������)
	BitonicSort(thread_id.x, dataIndex);
	//�X���b�h�����҂�
	GroupMemoryBarrierWithGroupSync();
	//�o�C�g�j�b�N�\�[�g���ŏI�X�e�b�v�ł���Ȃ�
	if (isBitonicFinal)	ConfigureIndirectArgs(thread_id.x, dataIndex);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	UpdateParticle
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//TODO : �������t���[���x�[�X�������藧���Ȃ��̂Ńf���^�x�[�X�ɑΉ�����
void UpdateIndividualParticle(uint data_offset)
{
	const uint aliveOffset = data_offset + PARTICLE_ELAPSED_TIME_OFFSET;
	const uint maxAliveOffset = data_offset + PARTICLE_ALIVE_TIME_OFFSET;
	//���݂̌o�ߎ��Ԏ擾
	float time = asfloat(particleData.Load(aliveOffset));
	//���Ԍo��
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
	//�ړ�
	//move = move * elapsedTime + 0.5f * power * (elapsedTime * elapsedTime);
	//��ƃC�R�[�� �W�J����ƌ����ʂ��ɂȂ�
	move = ((elapsedTime * power * 0.5f) + move) * elapsedTime;
	pos += move;
	move /= elapsedTime;
	//�K�p
	particleData.Store3(posOffset, asuint(pos));
	particleData.Store3(moveOffset, asuint(move));
}
[numthreads(LOCAL_THREAD_COUNT, 1, 1)]
void UpdateParticle(uint3 thread_id : SV_DispatchThreadID)
{
	for (int i = 0; i < THREAD_PER_COUNT; i++)
	{
		uint dataIndex = thread_id.x * THREAD_PER_COUNT + i;
		if (CHECK_PARTICLE_MAX_COUNT(dataIndex))return;
		dataIndex *= PARTICLE_DATA_SIZE;
		//�X�V
		UpdateIndividualParticle(dataIndex);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	�`��p�\����
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
struct GS_OUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
	float4 color : TEXCOORD1;
	int texIndex : TEXCOORD2;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	VertexShader
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//���_�V�F�[�_�[�̓X���[
GS_IN GPUParticleVS(GS_IN vs_in)
{
	return vs_in;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	GeometryShader
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GPUParticleGS(point GS_IN gs_in[1], inout TriangleStream<GS_OUT> triangle_stream)
{
	//�i�s��
	float nowRate = 1.0f - gs_in[0].elapsedTime / gs_in[0].aliveTime;
	float scale = lerp(gs_in[0].startScale, gs_in[0].endScale, nowRate);
	if (!IsFrustumRange(gs_in[0].pos, sqrt(scale*scale * 2)))return;
	float angle = lerp(gs_in[0].startRad, gs_in[0].endRad, nowRate);
	float invElapsedTime = gs_in[0].aliveTime - gs_in[0].elapsedTime;
	float alpha = 1.0f;
	if (invElapsedTime <= gs_in[0].fadeInTime)alpha = (float)invElapsedTime / gs_in[0].fadeInTime;
	else if (invElapsedTime >= gs_in[0].fadeOutTime)alpha = 1.0f - (float)(invElapsedTime - gs_in[0].fadeOutTime) / (float)(gs_in[0].aliveTime - gs_in[0].fadeOutTime);
	//���[���h�ϊ��s��쐬
	matrix wvp = GetIdentityMatrix();
	wvp._11_21 = float2(cos(angle), sin(angle));
	wvp._12_22 = float2(-sin(angle), cos(angle));
	//�r���{�[�h�s�񂩂�wvp�ϊ��s��쐬
	wvp = mul(transpose(billboardMat), wvp);
	wvp._14_24_34 = gs_in[0].pos;
	matrix vp = transpose(view);
	vp = mul(transpose(projection), vp);
	wvp = mul(vp, wvp);
	GS_OUT gs_out[4] = (GS_OUT[4])0;
	//���_�쐬
	gs_out[0].pos = mul(wvp, float4(0.5*scale, 0.5*scale, 0.0f, 1.0f));
	gs_out[1].pos = mul(wvp, float4(-0.5*scale, 0.5*scale, 0.0f, 1.0f));
	gs_out[2].pos = mul(wvp, float4(0.5*scale, -0.5*scale, 0.0f, 1.0f));
	gs_out[3].pos = mul(wvp, float4(-0.5*scale, -0.5*scale, 0.0f, 1.0f));
	//uv�ݒ�
	gs_out[0].tex = gs_in[0].uvPos;
	gs_out[1].tex = gs_in[0].uvPos + float2(gs_in[0].uvSize.x, 0.0f);
	gs_out[2].tex = gs_in[0].uvPos + float2(0.0f, gs_in[0].uvSize.y);
	gs_out[3].tex = gs_in[0].uvPos + gs_in[0].uvSize;
	//�A���t�@�l�ݒ�
	gs_out[0].color = float4(gs_in[0].color.rgb, alpha);
	gs_out[1].color = float4(gs_in[0].color.rgb, alpha);
	gs_out[2].color = float4(gs_in[0].color.rgb, alpha);
	gs_out[3].color = float4(gs_in[0].color.rgb, alpha);
	//�e�N�X�`���ݒ�
	gs_out[0].texIndex = gs_in[0].textureIndex;
	gs_out[1].texIndex = gs_in[0].textureIndex;
	gs_out[2].texIndex = gs_in[0].textureIndex;
	gs_out[3].texIndex = gs_in[0].textureIndex;
	//�X�g���[���ɏo��
	triangle_stream.Append(gs_out[0]);
	triangle_stream.Append(gs_out[1]);
	triangle_stream.Append(gs_out[2]);
	triangle_stream.Append(gs_out[3]);
	triangle_stream.RestartStrip();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	PixelShader
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float4 GPUParticlePS(GS_OUT gs_out) : SV_Target
{
	float4 color;
switch (gs_out.texIndex)
{
case 0:	color = txDiffuse0.Sample(samPoint, gs_out.tex); break;
case 1:	color = txDiffuse1.Sample(samPoint, gs_out.tex); break;
case 2:	color = txDiffuse2.Sample(samPoint, gs_out.tex); break;
case 3:	color = txDiffuse3.Sample(samPoint, gs_out.tex); break;
case 4:	color = txDiffuse4.Sample(samPoint, gs_out.tex); break;
case 5:	color = txDiffuse5.Sample(samPoint, gs_out.tex); break;
case 6:	color = txDiffuse6.Sample(samPoint, gs_out.tex); break;
case 7:	color = txDiffuse7.Sample(samPoint, gs_out.tex); break;
default:	color = float4(1.0f,1.0f,1.0f,1.0f);						  break;
}
return color * gs_out.color;
}
