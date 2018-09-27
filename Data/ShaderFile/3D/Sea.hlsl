//------------------------------------------------------------------------------------------------------
//
//		�w�b�_�[���
//
//------------------------------------------------------------------------------------------------------
//C++���ɂ�������`������
//#define __PARABOLOID__
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
cbuffer SeaInfo : register(b6) {
	float min : packoffset(c0.x);
	float max : packoffset(c0.y);
	float maxDevide : packoffset(c0.z);
	float hegihtBias : packoffset(c0.w);
	float time : packoffset(c1.x);
	float transparency : packoffset(c1.y);
};
cbuffer CubeMap:register(b7) {
	column_major float4x4 views[6];
	column_major float4x4 projectionCube;
	int isLighting;
}


//�F�e�N�X�`��
Texture2D txDiffuse: register(t0);
//�C�ʐ����p�e�N�X�`�� 
Texture2D txDisplacementMap: register(t1);
//�m�[�}���}�b�v
Texture2D txNormalMap: register(t3);
#ifdef __PARABOLOID__
//�o�Ȗʊ��}�b�v
Texture2DArray txParaboloid: register(t4);
#else
TextureCube txCube : register(t4);
#endif
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

// �o�̓p�b�`�萔�f�[�^�B
struct HS_TRI_OUT_DATA
{
	float edgeFactor[3] : SV_TessFactor;
	float insideFactor : SV_InsideTessFactor;
};

struct GS_CREATE_CUBE_OUT {
	float4 pos : SV_POSITION;
	float4 eyeVector : VECTOR0;
	float4 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};
#ifdef __PARABOLOID__
struct GS_OUT {
	float4 pos : SV_POSITION;
	float4 eyeVector : VECTOR0;
	float4 normal : NORMAL0;
	//float4 tangentSpaceLightDirection : NORMAL1;
	float2 tex : TEXCOORD0;
	float2 paraboloidTex0 : TEXCOORD1;
	float2 paraboloidTex1 : TEXCOORD2;
	float isFront : IS_FRONT;
};
#else
struct GS_OUT {
	float4 pos : SV_POSITION;
	float4 oldPos : OLD_POSITION;
	float4 eyeVector : VECTOR0;
	float4 oldEyeVector : VECTOR1;
	float4 normal : NORMAL0;
	float4 tangentLight : LIGHT0;
	float2 tex : TEXCOORD0;
};

#endif

//------------------------------------------------------------------------------------------------------
//
//		Common Function
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
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	return transpose(mat); // �]�u
}

//------------------------------------------------------------------------------------------------------
//
//		Vertex Shader
//
//------------------------------------------------------------------------------------------------------
//���}�b�v�쐻�p
//���}�b�v�쐬���̓e�b�Z���[�^�������g�p���Ȃ�
VS_OUT VS_CREATE_CUBE(VS_IN vs_in) {
	VS_OUT vs_out = (VS_OUT)0;
	//���[���h�ϊ�
	vs_out.pos = mul(vs_in.pos, world);
	vs_out.normal = mul(vs_in.normal, world);
	vs_out.tex = vs_in.tex;
	return vs_out;
}
//�C�p
//���̂܂܎��̃X�e�[�W�ɗ��� 
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
	//�ʂ̒��S�_�Z�o
	float3 center = (ip[0].pos.xyz + ip[1].pos.xyz + ip[2].pos.xyz) / 3.0f;
	//���S�_�Ƃ̋���
	float dist = length(cpos.xyz - center);
	//�ŏ������ƍő勗���̕ۏ�
	dist = clamp(dist, min, max);
	//�ŏ��`�ő勗���ł̔䗦�Z�o
	float ratio = (dist - min) / (max - min);
	//�ő啪���������ł̔䗦��p�����������K��
	devide = (1.0f - ratio) * maxDevide + 1;
	//�������̒�`
	Output.edgeFactor[0] = devide;
	Output.edgeFactor[1] = devide;
	Output.edgeFactor[2] = devide;
	Output.insideFactor = devide;
	return Output;
}
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
	//�f�B�X�v���[�X�����g 
	float4 height = txDisplacementMap.SampleLevel(samLinear, output.tex, 0);

	//domain��UV�l ����ɂ��ق��O�_�̃R���g���[���|�C���g������W���Z�o
	//�ʒu�Z�o
	float3 pos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;
	output.pos = float4(pos, 1.0f);
	//�@��(���ۂɋ��߂�̂�GS)
	output.normal = normalize(patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
	//output.normal.xyz = normalize(normal);
	//output.normal = normalize(mul(output.normal, world));
	//���ۂɒ��_�𓮂�������
	output.oldPos = output.pos + output.normal * (sin(time + output.pos.x) / 3.0f);
	output.pos += output.normal * height;
	output.pos += output.normal * (sin(time + output.pos.x) / 3.0f);
	//output.tex.x = (sin(time + output.tex.x) / 3.14 + 1.0f)*0.5f;
	//���[���h�ϊ���
	output.pos = mul(output.pos, world);
	output.oldPos = mul(output.oldPos, world);
	output.eyeVector = normalize(cpos - output.pos);
	output.oldEyeVector = normalize(cpos - output.oldPos);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	return output;
}

//------------------------------------------------------------------------------------------------------
//
//		Geometry Shader
//
//------------------------------------------------------------------------------------------------------
//���}�b�v�쐻�p
[maxvertexcount(18)]
void GS_CREATE_CUBE(triangle VS_OUT gs_in[3], inout TriangleStream<GS_CREATE_CUBE_OUT> stream) {
	for (int i = 0; i < 6; i++) {
		GS_CREATE_CUBE_OUT ret = (GS_CREATE_CUBE_OUT)0;
		//�����_�[�^�[�Q�b�g�̎w��
		ret.renderTargetIndex = i;
		//�X�N���[���֕ϊ�
		//[unroll]
		for (int j = 0; j < 3; j++) {
			ret.eyeVector = normalize(cpos - gs_in[j].pos);
			ret.pos = mul(gs_in[j].pos, views[i]);
			ret.pos = mul(ret.pos, projectionCube);
			ret.normal = gs_in[j].normal;
			ret.tex = gs_in[j].tex;
			//�o��
			stream.Append(ret);
		}
		//���߂̔��s
		stream.RestartStrip();
	}
}
//�C�p
[maxvertexcount(9)]
void GS(triangle GS_OUT gs_in[3], inout TriangleStream<GS_OUT> stream) {
	//�d�S�Z�o
	float3 center = (gs_in[0].pos + gs_in[1].pos + gs_in[2].pos) / 3.0f;
	float3 cvVec[3] = (float3[3])0;
	for (int i = 0; i < 3; i++) {
		cvVec[i] = normalize(gs_in[i].pos - center);
	}
	//�ʖ@���Z�o
	/*float3 vec0 = gs_in[0].pos - gs_in[1].pos;
	float3 vec1 = gs_in[2].pos - gs_in[1].pos;
	float3 normal = normalize(cross(vec0, vec1));*/
	//TODO : �@���}�b�v���������肷�邽�߂ɖ{�C�̐ڋ�ԋ��߂ɍs���B
	//float4 tangent = float4(CalcTangent(normal), 0.0f);
	//float4 binormal = float4(CalcBinormal(normal), 0.0f);
	//float4 tangentSpaceLightDirection = mul(lightDirection, InvTangentMatrix(tangent, binormal, normal));
	GS_OUT gs_out[3] = (GS_OUT[3])0;
	//�ڋ�ԎZ�o
	//5��������3������
	float3 cp0[3] = (float3[3])0;
	cp0[0] = float3(gs_in[0].oldPos.x, gs_in[0].tex.x, gs_in[0].tex.y);
	cp0[1] = float3(gs_in[0].oldPos.y, gs_in[0].tex.x, gs_in[0].tex.y);
	cp0[2] = float3(gs_in[0].oldPos.z, gs_in[0].tex.x, gs_in[0].tex.y);
	float3 cp1[3] = (float3[3])0;
	cp1[0] = float3(gs_in[1].oldPos.x, gs_in[1].tex.x, gs_in[1].tex.y);
	cp1[1] = float3(gs_in[1].oldPos.y, gs_in[1].tex.x, gs_in[1].tex.y);
	cp1[2] = float3(gs_in[1].oldPos.z, gs_in[1].tex.x, gs_in[1].tex.y);
	float3 cp2[3] = (float3[3])0;
	cp2[0] = float3(gs_in[2].oldPos.x, gs_in[2].tex.x, gs_in[2].tex.y);
	cp2[1] = float3(gs_in[2].oldPos.y, gs_in[2].tex.x, gs_in[2].tex.y);
	cp2[2] = float3(gs_in[2].oldPos.z, gs_in[2].tex.x, gs_in[2].tex.y);
	//���ʂ���t�u���W�Z�o
	float u[3] = (float[3])0;
	float v[3] = (float[3])0;
	for (int i = 0; i < 3; i++) {
		float3 v0 = cp1[i] - cp0[i];
		float3 v1 = cp2[i] - cp1[i];
		float3 crossVector = cross(v0, v1);
		//�k�ނ����ǂǂ����悤���Ȃ�
		//if (crossVector.x == 0.0f);
		u[i] = -crossVector.y / crossVector.x;
		v[i] = -crossVector.z / crossVector.x;
	}
	float4 tangent = normalize(float4(u[0], u[1], u[2], 0.0f));
	float4 binormal = normalize(float4(v[0], v[1], v[2], 0.0f));
	float4 normal = float4(normalize(cross(tangent, binormal)), 0.0f);
	float4x4 tangentMatrix = InvTangentMatrix(tangent, binormal, normal);
	float4 tangentLight = normalize(mul(lightDirection, tangentMatrix));
	for (int i = 0; i < 3; i++) {
		gs_out[i].pos = gs_in[i].pos;
		//gs_out[i].normal.xyz = gs_in[i].normal;
		gs_out[i].normal.xyz = normal;
		gs_out[i].tangentLight = tangentLight;
		gs_out[i].eyeVector = normalize(mul(gs_in[i].oldEyeVector, tangentMatrix));
		//gs_out[i].tangentSpaceLightDirection = tangentSpaceLightDirection;
		gs_out[i].tex = gs_in[i].tex;
#ifdef __PARABOLOID__
		float3 ray = normalize(-gs_out[i].eyeVector).xyz;
		float3 ref = reflect(ray, gs_out[i].normal.xyz);
		gs_out[i].paraboloidTex0.x = 0.5f*(1 + ref.x / (1 + ref.z));
		gs_out[i].paraboloidTex0.y = 0.5f*(1 - ref.y / (1 + ref.z));
		gs_out[i].paraboloidTex1.x = 0.5f*(1 - ref.x / (1 - ref.z));
		gs_out[i].paraboloidTex1.x = 0.5f*(1 - ref.y / (1 - ref.z));
		gs_out[i].isFront = ref.z + 0.5f;
#endif
		//�X�g���[���ɏo��
		stream.Append(gs_out[i]);
	}
	stream.RestartStrip();
}

//------------------------------------------------------------------------------------------------------
//
//		Pixel Shader
//
//------------------------------------------------------------------------------------------------------
//���}�b�v�쐻�p
float4 PS_CREATE_CUBE(GS_CREATE_CUBE_OUT ps_in) :SV_Target{
	//�F���
	float4 diffuse = txDiffuse.Sample(samLinear, ps_in.tex);
	///�S�V�F�[�_�[���ʂ̌��ʂɂȂ邽�߁A�œK��������͂�
	if (!isLighting) return diffuse;
	//���C�e�B���O
	float3 lambert = saturate(dot(ps_in.normal, lightDirection));
	//���˃x�N�g���擾
	float3 reflectValue = normalize(reflect(lightDirection, ps_in.normal));
	//�X�؃L�����Z�o
	float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	//�ŏI����
	return float4((diffuse.rgb*lambert + specular), diffuse.a);
}
//�C�p
float4 PS(GS_OUT ps_in) :SV_Target{
	//�F���
	float4 diffuse = txDiffuse.Sample(samLinear, ps_in.tex);
#ifdef __PARABOLOID__//�o�Ȗʊ��}�b�v
	//���}�b�v
	float4 front = txParaboloid.Sample(samLinear, float3(ps_in.paraboloidTex0,0));
	float4 back = txParaboloid.Sample(samLinear, float3(ps_in.paraboloidTex1,1));
	float4 environment;
	if (ps_in.isFront > 0.5f) {
		environment = front;
	}
	else {
		environment = back;
	}
	//���C�e�B���O
	float3 lambert = saturate(dot(ps_in.normal, lightDirection));
	//���˃x�N�g���擾
	float3 reflectValue = normalize(reflect(lightDirection, ps_in.normal));
	//�X�؃L�����Z�o
	float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	return float4(environment.rgb, transparency);
	return float4((environment.rgb*lambert + specular), diffuse.a*transparency);
	return float4((diffuse.rgb*lambert + specular * environment.rgb), diffuse.a*transparency);
#else//���̐�L���[�u�}�b�v
	//return float4(ps_in.normal.xyz, 1.0f);
	//�@���}�b�v�ǂݍ���
	float3 normalColor = txNormalMap.Sample(samLinear, ps_in.tex);
	float3 normalVector = normalize(2.0f * normalColor - 1.0f);
	//���}�b�v�ǂݍ���
	float3 reflectRay = reflect(ps_in.eyeVector.xyz, normalVector.xyz);
	float4 environment = txCube.Sample(samLinear, reflectRay);
	//���C�e�B���O
	float3 lambert = saturate(dot(normalVector, ps_in.tangentLight));
	//���˃x�N�g���擾
	float3 reflectValue = /*normalize*/(reflect(ps_in.tangentLight, normalVector));
	//�X�؃L�����Z�o
	float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	return float4(environment.rgb, transparency);
	return float4((diffuse.rgb*lambert + specular * environment), diffuse.a*transparency);
#endif
	//if (!test)return float4((diffuse.rgb*lambert + specular), diffuse.a*transparency);
	//else return float4(1.0f,1.0f,1.0f,1.0f);
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
