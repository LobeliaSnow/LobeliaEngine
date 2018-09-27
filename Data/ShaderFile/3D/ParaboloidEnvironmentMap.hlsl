//�J������� 
cbuffer View : register(b0) {
	column_major float4x4 view;
	column_major float4x4 projection;
	column_major float4x4 billboardMat;
	float4 cpos;
	struct Frustum {
		float4 center[6];
		float4 normal[6];
	} frustum;
};


//���[���h�ϊ��s��
cbuffer World : register(b1) {
	column_major float4x4 world;
};
//���p
cbuffer Environment : register(b4)
{
	//���K������Ă��܂�
	float4 lightDirection;
	float4 ambientColor;
	float4 fogInfo;
}

cbuffer EnvironmentInfo : register(b7) {
	//�o�ȖʂȂ̂œ��
	column_major float4x4 views[2];
	column_major float4x4 projectionParaboloid;
	float zNear;
	float zFar;
};
cbuffer DebugInfo : register(b8) {
	//�o�ȖʂȂ̂œ��
	int textureIndex : packoffset(c0.x);
};
//�F�e�N�X�`��
Texture2D txDiffuse: register(t0);
//�o�Ȗʊ��}�b�v
Texture2DArray txParaboloid: register(t4);
SamplerState samLinear : register(s0);

//���͗p�\���� 
struct VS_IN {
	float4 pos : POSITION0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	uint4 clusterIndex : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

struct VS_CREATE_PARABOLOID_OUT {
	float4 pos : SV_POSITION;
	float4 eyeVector : VECTOR0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
};

struct GS_CREATE_PARABOLOID_OUT {
	float4 pos : SV_POSITION;
	float4 eyeVector : VECTOR0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	//�����p�X�p���{���C�h�p
	//�����_�[�^�[�Q�b�g���w��
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};
//�f�o�b�O�p
struct VS_IN_TEX
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};

//�f�o�b�O�p�X�v���C�g�\����
struct PS_IN_TEX
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};

//�����̈ʒu�̓��[���h�ϊ��ς݂̂��̂��w��
float4 Paraboloid(float4 position, int render_id) {
	float4 ret = (float4)0;
	float4 pos = mul(position, views[render_id]);
	//�����擾
	float l = length(pos);
	//������Ɖ�����Ă邩�킩���B
	//��ōl����
	ret.xy = pos.xy*l*(l - pos.z) / dot(pos.xy, pos.xy);
	ret.z = (l - zNear)*zFar / (zFar - zNear);
	//��a��if ���������@���Ȃ����l����
	if (ret.z >= 0.0f) {//�`��Ώۂ������̕\�ʂ������ꍇ
		ret.z = (l - zNear)*zFar / (zFar - zNear);
		ret.w = l;
	}
	else {//�`��Ώۂ������̗��ʂ������ꍇ
		ret.z = 0.0f;
		//����������
		ret.w = -0.00001f;
	}
	return ret;
}

//���}�b�v�쐻�p
VS_CREATE_PARABOLOID_OUT VS_CREATE_PARABOLOID(VS_IN vs_in) {
	VS_CREATE_PARABOLOID_OUT vs_out = (VS_CREATE_PARABOLOID_OUT)0;
	vs_out.pos = mul(vs_in.pos, world);
	vs_out.eyeVector = normalize(cpos - vs_out.pos);
	vs_out.normal = mul(vs_in.normal, world);
	vs_out.tex = vs_in.tex;
	//vs_out.renderTargteIndex = 0;
	return vs_out;
}

//�f�o�b�O�p
PS_IN_TEX VS_DEBUG_SPRITE(VS_IN_TEX vs_in)
{
	PS_IN_TEX output;
	output.pos = vs_in.pos;
	output.col = vs_in.col;
	output.tex = vs_in.tex;
	return output;
}

[maxvertexcount(6)]
void GS_CREATE_PARABOLOID(triangle VS_CREATE_PARABOLOID_OUT gs_in[3], inout TriangleStream<GS_CREATE_PARABOLOID_OUT> stream) {
	for (int i = 0; i < 2; i++) {
		GS_CREATE_PARABOLOID_OUT gs_out = (GS_CREATE_PARABOLOID_OUT)0;
		gs_out.renderTargetIndex = i;
		for (int j = 0; j < 3; j++) {
			gs_out.pos = Paraboloid(gs_in[j].pos, i);
			gs_out.pos = mul(gs_out.pos, projectionParaboloid);
			gs_out.normal = gs_in[j].normal;
			gs_out.tex = gs_in[j].tex;
			//�o��
			stream.Append(gs_out);
		}
		//���߂̔��s
		stream.RestartStrip();
	}
}

//���}�b�v�쐻�p
float4 PS_CREATE_PARABOLOID(GS_CREATE_PARABOLOID_OUT ps_in) :SV_Target{
	//�F���
	float4 diffuse = txDiffuse.Sample(samLinear, ps_in.tex);
	return float4(diffuse.rgb, diffuse.a);
	//���C�e�B���O
	float3 lambert = saturate(dot(ps_in.normal, lightDirection));
	//���˃x�N�g���擾
	float3 reflectValue = normalize(reflect(lightDirection, ps_in.normal));
	//�X�؃L�����Z�o
	float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	return float4((diffuse.rgb*lambert + specular), diffuse.a);
}
//�f�o�b�O�p�X�v���C�g�\��
float4 PS_DEBUG_SPRITE(PS_IN_TEX ps_in) :SV_Target{
	return float4(txParaboloid.Sample(samLinear, float3(ps_in.tex,textureIndex)).rgb,1.0f);
}