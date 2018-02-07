struct Skin
{
	float4 pos;
	float3 normal;
};

struct VS_IN
{
	float4 pos : POSITION0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	uint4 clusterIndex : BONEINDEX;
	float4 weights : BONEWEIGHT;
};
struct VS_NOSKIN_INDIRECT_IN
{
	float4 pos : POSITION0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	uint InstanceId : SV_InstanceID; // �C���X�^���X�h�c
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	float4 eyeVector : TEXCOORD1;
	float4 color : TEXCOORD2;
	//�m�[�}���}�b�v�p
	float4 tangentLightDirection : TEXCOORD3;
	//�V���h�E�}�b�v�p
	float4 lightTex: TEXCOORD4;
	float4 lightViewPos : TEXCOORD5;
};

struct VS_INSTANCING_IN
{
	float4 pos : POSITION0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	column_major float4x4 world : MATRIX_I;
	uint InstanceId : SV_InstanceID; // �C���X�^���X�h�c
};

struct VS_INSTANCING_ANIMATION_IN
{
	float4 pos : POSITION0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	uint4 clusterIndex : BONEINDEX;
	float4 weights : BONEWEIGHT;
	column_major float4x4 world : MATRIX_I;
	uint InstanceId : SV_InstanceID; // �C���X�^���X�h�c
};

struct PS_IN_DEPTH
{
	float4 pos : SV_POSITION;
	float4 depth : POSITION0;
	float2 tex : TEXCOORD0;
};