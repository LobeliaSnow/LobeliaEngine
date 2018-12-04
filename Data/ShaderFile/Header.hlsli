cbuffer View : register(b0)
{
	column_major float4x4 view;
	column_major float4x4 previousView;
	column_major float4x4 projection;
	column_major float4x4 previousProjection;
	column_major float4x4 billboardMat;
	//�r���[+�v���W�F�N�V�����s��̋t�s��
	column_major float4x4 inverseViewProjection;
	float4 cpos;
	struct Frustum
	{
		float4 center[6];
		float4 normal[6];
	} frustum;
};

cbuffer World : register(b1)
{
	column_major float4x4 world;
	int useAnimation;
};

cbuffer Material : register(b2)
{
	float4 diffuse :packoffset(c0);
	float4 ambient :packoffset(c1);
	float4 specular :packoffset(c2);
	float4 texColor :packoffset(c3);
	int useNormalMap :packoffset(c4.x);
	int useSpecularMap :packoffset(c4.y);
	int useEmission :packoffset(c4.z);
}

cbuffer Bone : register(b3)
{
	//1mesh�ӂ�255�{�̃{�[��
	column_major float4x4 keyFrames[256];
}

cbuffer Environment : register(b4)
{
	//���K������Ă��܂�
	float4 lightDirection :packoffset(c0.x);
	float4 ambientColor :packoffset(c1.x);
	float3 fogColor :packoffset(c2.x);
	float fogBegin :packoffset(c2.w);
	float fogEnd :packoffset(c3.x);
	float density : packoffset(c3.y);
	int useLinearFog :packoffset(c3.z);
}

cbuffer GaussianFilterInfo : register(b5)
{
	//�T�[�t�F�X�̃T�C�Y
	float width : packoffset(c0.x);
	float height : packoffset(c0.y);
}

//0~9�܂Ńt�@�C������ǂݍ��񂾃e�N�X�`�����
Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txSpecular : register(t2);
Texture2D txEmission : register(t3);

//10~�����_�[�^�[�Q�b�g����̓���
Texture2D rtDiffuse : register(t10);
Texture2D rtSpecular : register(t11);

//20~�C���X�^���V���O�����_���p
Texture2D txDiffuse0 : register(t20);
Texture2D txDiffuse1 : register(t21);
Texture2D txDiffuse2 : register(t22);
Texture2D txDiffuse3 : register(t23);
Texture2D txDiffuse4 : register(t24);
Texture2D txDiffuse5 : register(t25);
Texture2D txDiffuse6 : register(t26);
Texture2D txDiffuse7 : register(t27);


SamplerState samLinear : register(s0);

static const float PI = 3.141592653589793f;
