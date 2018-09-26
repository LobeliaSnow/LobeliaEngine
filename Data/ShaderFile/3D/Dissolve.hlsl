#include "../Header.hlsli"
#include "Header3D.hlsli"

//---------------------------------------------------------------------------------------------------------------
//�֐�
//---------------------------------------------------------------------------------------------------------------
//�e�N�X�`���T���v�����O
inline float4 GetDiffuseMap(float2 uv)
{
	return txDiffuse.Sample(samLinear, uv);
}
inline float4 LightingBias(float3 normalized_normal, float3 normalized_light_direction)
{
	float4 ret = saturate(dot(normalized_normal, normalized_light_direction.xyz));
	ret.a = 1.0f;
	return ret;
}
inline float4 SpecularCalc(float2 uv, float3 normalized_normal, float3 normalized_eye_vector, float3 normalized_light_direction, float3 light_bias)
{
	//�t�H��
	float3 reflect = normalize(light_bias.xyz * normalized_normal - normalized_light_direction.xyz);
	//�X�y�L�����}�b�v�l���̃X�y�L�����l�v�Z
	//���󒼒l�����Apow�̑������ŋ�����ς��邱�Ƃ��ł���
	return pow(dot(reflect, normalized_eye_vector), 4);
}

cbuffer DissolveInfo : register(b7) {
	//臒l
	float threshold : packoffset(c0.x);
};
//�f�B�]���u�p
Texture2D txDissolveMap: register(t3);

//�s�N�Z���V�F�[�_�[�̍����ւ������ŗǂ�
float4 PS(PS_IN ps_in) : SV_Target {
	//�f�B�]���u�p�̃e�N�X�`���̒l���t�F�b�`
	float dissolve = txDissolveMap.Sample(samLinear, ps_in.tex);
	//臒l��������Ă�����`�悵�Ȃ�
	if (dissolve <= threshold)return float4(0.0f,0.0f,0.0f,0.0f);
	//�t�H��
	float3 normal = normalize(ps_in.normal);
	float3 viewDir = normalize(ps_in.eyeVector);
	float4 normalLight = LightingBias(normal, lightDirection.xyz);
	//�X�y�L�����}�b�v�l���̃X�y�L�����l�v�Z
	float4 specular = SpecularCalc(ps_in.tex, normal, viewDir, lightDirection.xyz, normalLight.xyz);
	float4 diffuseColor = GetDiffuseMap(ps_in.tex) * texColor * ambientColor;
	normalLight.a = 1.0f;
	
	return diffuseColor * normalLight + specular;
}