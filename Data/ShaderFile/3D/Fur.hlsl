//------------------------------------------------------------------------------------------------------
//
//		�w�b�_�[���
//
//------------------------------------------------------------------------------------------------------
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
//�C�ʑ���p�萔�o�b�t�@ 
cbuffer FurInfo : register(b6) {
	float min : packoffset(c0.x);
	float max : packoffset(c0.y);
	float maxDevide : packoffset(c0.z);
	float furLength : packoffset(c0.w);
};

//�F�e�N�X�`��
Texture2D txDiffuse: register(t0);
SamplerState samLinear : register(s0);

//���͗p�\���� 
struct VS_IN {
	float4 pos : POSITION0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	uint4 clusterIndex : BONEINDEX;
	float4 weights : BONEWEIGHT;
};
struct VS_OUT {
	float4 pos : SV_POSITION;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
};
struct GS_OUT {
	float4 pos : SV_POSITION;
	float4 eyeVector : VECTOR0;
	float4 normal : NORMAL0;
	float4 tangentSpaceLightDirection : NORMAL1;
	float2 tex : TEXCOORD0;
	float alpha : ALPHA;
};

// �o�̓p�b�`�萔�f�[�^�B
struct HS_TRI_OUT_DATA
{
	float edgeFactor[3] : SV_TessFactor;
	float insideFactor : SV_InsideTessFactor;
};

//------------------------------------------------------------------------------------------------------
//
//		Vertex Shader
//
//------------------------------------------------------------------------------------------------------
//���̂܂�HullShader�ɗ��� 
VS_OUT VS(VS_IN vs_in) {
	VS_OUT vs_out = (VS_OUT)0;
	vs_out.pos = vs_in.pos;
	vs_out.normal = vs_in.normal;
	vs_out.tex = vs_in.tex;
	return vs_out;
}

//------------------------------------------------------------------------------------------------------
//
//		Hull Shader
//
//------------------------------------------------------------------------------------------------------
// �p�b�`�萔�֐� 
HS_TRI_OUT_DATA CalcTriConstants(InputPatch<VS_OUT, 3> ip, uint PatchID : SV_PrimitiveID) {
	HS_TRI_OUT_DATA Output;
	//�����ɉ������������̎Z�o
	float devide = 0.0f;
	//float dist = length(cpos.xyz - ip[0].pos.xyz);
	//dist += length(cpos.xyz - ip[1].pos.xyz);
	//dist += length(cpos.xyz - ip[2].pos.xyz);
	//dist /= 3.0f;
	//if (dist < min)  dist = min;
	//else if (dist > max)  dist = max;
	//float x = (dist - min) / (max - min);
	//devide = (1 - x) * maxDevide + 1;

	devide = 32;
	//�������̒�`
	Output.edgeFactor[0] = devide;
	Output.edgeFactor[1] = devide;
	Output.edgeFactor[2] = devide;
	Output.insideFactor = devide;
	return Output;
}
//�n���V�F�[�_�[�̓X���[
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcTriConstants")]
VS_OUT HS(InputPatch<VS_OUT, 3> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
	return ip[i];
}

//------------------------------------------------------------------------------------------------------
//
//		Domain Shader
//
//------------------------------------------------------------------------------------------------------
[domain("tri")]
GS_OUT DS(HS_TRI_OUT_DATA input, float3 domain : SV_DomainLocation, const OutputPatch<VS_OUT, 3> patch)
{
	GS_OUT output = (GS_OUT)0;
	//�e�N�X�`�����W�Z�o
	output.tex = patch[0].tex * domain.x + patch[1].tex * domain.y + patch[2].tex * domain.z;
	//domain��UV�l ����ɂ��ق��O�_�̃R���g���[���|�C���g������W���Z�o
	//�ʒu�Z�o
	float3 pos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;
	output.pos = float4(pos, 1.0f);
	//output.tex.x = (sin(time + output.tex.x) / 3.14 + 1.0f)*0.5f;
	//�@�����(���ۂɋ��߂�̂�GS)
	output.normal = normalize(patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
	output.normal = normalize(mul(output.normal, world));
	//���[���h�ϊ���
	output.pos = mul(output.pos, world);
	output.eyeVector = normalize(cpos - output.pos);
	//output.pos = mul(output.pos, view);
	//output.pos = mul(output.pos, projection);

	return output;
}

//------------------------------------------------------------------------------------------------------
//
//		Geometry Shader
//
//------------------------------------------------------------------------------------------------------
//�@���̎Z�o�Ɏg�p
inline float3 CalcTangent(float3 normal)
{
	static float3 up = normalize(float3(0, 1, 1));
	return cross(up, normal);
}
inline float3 CalcBinormal(float3 normal)
{
	static float3 right = float3(1, 0, 0);
	return cross(normal, right);
}
inline float4x4 InvTangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
	float4x4 mat =
	{
		{ float4(tangent, 0.0f) },
	{ float4(binormal, 0.0f) },
	{ float4(normal, 0.0f) },
	{ 0, 0, 0, 1 }
	};
	return transpose(mat); // �]�u
}
[maxvertexcount(2)]
void GS(triangle GS_OUT gs_in[3], inout LineStream<GS_OUT> line_stream)
{
	//�@���Z�o
	float3 vec0 = gs_in[0].pos - gs_in[1].pos;
	float3 vec1 = gs_in[2].pos - gs_in[1].pos;
	float3 normal = normalize(cross(vec0, vec1));
	GS_OUT gs_out[2] = (GS_OUT[2])0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			gs_out[j].pos.xyz = gs_in[i].pos.xyz + normal * j * furLength;
			gs_out[j].pos.w = 1.0f;
			gs_out[j].pos = mul(gs_out[j].pos, view);
			gs_out[j].pos = mul(gs_out[j].pos, projection);
			gs_out[j].normal.xyz = normal;
			gs_out[j].eyeVector = gs_in[i].eyeVector;
			gs_out[j].tex = gs_in[i].tex;
			gs_out[j].alpha = (1 - j) * 1.0f;
			//�X�g���[���ɏo��
			line_stream.Append(gs_out[j]);
		}
		line_stream.RestartStrip();
	}
}

//------------------------------------------------------------------------------------------------------
//
//		Pixel Shader
//
//------------------------------------------------------------------------------------------------------
float4 PS(GS_OUT ps_in) :SV_Target{
	//�F���
	float4 diffuse = txDiffuse.Sample(samLinear, ps_in.tex);
	//���C�e�B���O
	float3 lambert = saturate(dot(ps_in.normal, lightDirection));
	//return float4(1.0f,1.0f,1.0f,1.0f);
	return float4(diffuse.rgb*lambert, ps_in.alpha);
	////���˃x�N�g���擾
	//float3 reflectValue = normalize(reflect(lightDirection, ps_in.normal));
	////�X�؃L�����Z�o
	//float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	//return float4((diffuse.rgb*lambert + specular), diffuse.a*0.5f);
	//float3 normalMap = txNormalMap.Sample(samLinear, ps_in.tex);
	//normalMap = normalize(2 * normalMap - 1.0f);
	//return float4(normalMap, 1.0f);
	//return float4(normalize(ps_in.normal.xyz),1.0f);
	//return float4(ps_in.tangentSpaceLightDirection.xyz, 0.0f);
	//float lambert = dot(normalMap, ps_in.tangentSpaceLightDirection);
	//float3 reflectValue = normalize(lightDirection.xyz - ps_in.normal.xyz*2.0f*dot(lightDirection,ps_in.normal));
	//return float4(reflectValue, 1.0f);
	//return float4(specular, 1.0f);
	//return lambert;
}