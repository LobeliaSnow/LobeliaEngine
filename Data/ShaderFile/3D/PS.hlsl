#include "../Header.hlsli"
#include "Header3D.hlsli"

struct Normal
{
	float3 normal;
	float3 lightDirection;
};

//---------------------------------------------------------------------------------------------------------------
//�֐�
//---------------------------------------------------------------------------------------------------------------
//�e�N�X�`���T���v�����O
inline float4 GetDiffuseMap(float2 uv)
{
	return txDiffuse.Sample(samLinear, uv);
}
inline float4 GetNormalMap(float2 uv)
{
	return txNormal.Sample(samLinear, uv);
}
inline float4 GetSpecularMap(float2 uv)
{
	return txSpecular.Sample(samLinear, uv);
}

//fog�W���Z�o
inline float IndexFogCalc(float4 pos)
{
	static const float e = 2.71828f;
	//����
	float dist = pos.z * pos.w;
	//���x
	float density = fogInfo.w;
	//�t�H�O�t�@�N�^�[
	float f = pow(e, -dist * density);
	//�t�H�O�̗�
	f *= 1.0f;
	f = saturate(f);

	return f;
}
//fog�K�p
inline float4 GetFogColor(float4 diffuseColor, float fog)
{
	return float4(fog * diffuseColor.xyz + (1.0f - fog) * fogInfo.rgb, diffuseColor.a);
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
	return pow(dot(reflect, normalized_eye_vector), 8) * GetSpecularMap(uv);
}
//---------------------------------------------------------------------------------------------------------------
//���̐�̓t�H���[�h�V�F�[�f�B���O�p
//---------------------------------------------------------------------------------------------------------------
interface PSInterfaceForward
{
	float4 GetColor(PS_IN ps_in);
};
class Lambert : PSInterfaceForward
{
	float4 GetColor(PS_IN ps_in)
	{
		return GetDiffuseMap(ps_in.tex) * texColor * ps_in.color * ambientColor;
	}
};
class Fog : PSInterfaceForward
{
	float4 GetColor(PS_IN ps_in)
	{
		float4 diffuseColor = GetDiffuseMap(ps_in.tex) * texColor * ps_in.color * ambientColor;
		float fog = IndexFogCalc(ps_in.pos);
		return GetFogColor(diffuseColor, fog);
	}
};
class Phong : PSInterfaceForward
{
	float4 GetColor(PS_IN ps_in)
	{
		//�t�H��
		float3 normal = normalize(ps_in.normal);
		float3 viewDir = normalize(ps_in.eyeVector);
		float4 normalLight = LightingBias(normal, lightDirection.xyz);
		//�X�y�L�����}�b�v�l���̃X�y�L�����l�v�Z
		float4 specular = SpecularCalc(ps_in.tex, normal, viewDir, lightDirection.xyz, normalLight.xyz);
		float4 diffuseColor = GetDiffuseMap(ps_in.tex) * texColor * ambientColor;
		//normalLight *= 0.5f;
		//normalLight += 0.3f;
		normalLight.a = 1.0f;
		return diffuseColor * normalLight + specular;
	}
};
//---------------------------------------------------------------------------------------------------------------
//���̐�̓f�B�t�@�[�h�V�F�[�f�B���O�p
//---------------------------------------------------------------------------------------------------------------
struct BlumePhong
{
	float4 diffuse : SV_Target0;
	float4 specular : SV_Target1;
};
interface BlumePSInterface
{
	Normal GetNormal(PS_IN ps_in);
};
class UseNormalMapBlume : BlumePSInterface
{
	Normal GetNormal(PS_IN ps_in)
	{
		Normal ret = (Normal)0;
		float3 normalMap = GetNormalMap(ps_in.tex);
		ret.normal = normalize(2 * normalMap - 1.0f);
		ret.lightDirection = ps_in.tangentLightDirection.xyz;
		return ret;
	}
};
class NoUseNormalMapBlume : BlumePSInterface
{
	Normal GetNormal(PS_IN ps_in)
	{
		Normal ret = (Normal)0;
		ret.normal = normalize(ps_in.normal);
		ret.lightDirection = lightDirection.xyz;
		return ret;
	}
};

BlumePSInterface normalInterface;

BlumePhong PreBlumePhongPS(PS_IN ps_in)
{
	BlumePhong ret = (BlumePhong)0;
	float3 viewDir = normalize(ps_in.eyeVector);
	Normal normal = normalInterface.GetNormal(ps_in);
	float4 normalLight = max(0.1f, LightingBias(normal.normal, normal.lightDirection));
	ret.diffuse = GetDiffuseMap(ps_in.tex) * texColor * ambientColor;
	//�����̔{���Ńu���[���̋������ϓ�
	ret.specular = SpecularCalc(ps_in.tex, normal.normal, viewDir, normal.lightDirection, normalLight.xyz);
	ret.specular.xyz *= ret.diffuse.xyz;
	ret.specular *= 1.0;
	ret.specular.a = 1.0f;
	ret.diffuse.xyz *= normalLight.xyz;
	//ret.specular = ret.diffuse - 0.3f;
	//ret.diffuse.xyz += ret.specular.xyz;

	return ret;
}


//struct MRT
//{
//    float4 diffuse : SV_Target0;
//    float4 position : SV_Target1;
//    float4 normal : SV_Target2;
//    float4 specular : SV_Target3;
//};
//class PSInterfaceDeferred/* : PSInterfaceForward*/
//{
//    MRT WriteTexture(PS_IN ps_in)
//    {
//        MRT mrt = (MRT) 0;
//        mrt.diffuse = GetDiffuseMap(ps_in.tex);
//        mrt.position = ps_in.pos;
//		//���K��
//        float3 normal = normalize(ps_in.normal);
//        float3 lightDir = normalize(lightDirection);
//        float3 viewDir = normalize(ps_in.eyeVector);
//		//���C�e�B���O
//        float4 normalLight = LightingBias(normal);
//		//�X�y�L�����Z�o
//        mrt.specular = SpecularCalc(ps_in.tex, normal, viewDir, lightDir, normalLight.xyz);
//		//��قǃm�[�}���}�b�v���ʎZ�o
//        mrt.normal = float4(normal, 0);
//    }
//};

PSInterfaceForward ps;

//---------------------------------------------------------------------------------------------------------------
//�{��
//---------------------------------------------------------------------------------------------------------------
float4 Main3D(PS_IN ps_in) : SV_Target0
{
	return ps.GetColor(ps_in);
}

float4 MainPolygon(PS_IN_POLYGON ps_in) : SV_Target0
{
	return GetDiffuseMap(ps_in.tex) * ps_in.color * ambientColor;
}
