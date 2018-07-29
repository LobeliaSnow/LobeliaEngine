#include "../Header.hlsli"
#include "Header3D.hlsli"

//TODO : 動的シェーダーリンケージで管理

//平行光源
inline float DirectionalLight(float4 arg_nor, float3 light_dir, float4x4 world)
{
    float3 normal = mul((float3) arg_nor, (float3x3) world);
    normal = normalize(normal);
    return max(dot(normal, light_dir), 0.0f) * 0.5f + 0.3f;
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
    return transpose(mat); // 転置
}
inline float3 CalcTangent(float3 normal)
{
    static float3 up = float3(0, 1, 0);
    return cross(up, normal);
}
inline float3 CalcBinormal(float3 normal)
{
    static float3 right = float3(1, 0, 0);
    return cross(normal, right);
}
matrix FetchBoneMatrix(uint iBone)
{
    return keyFrames[iBone];
}

Skin SkinVertex(VS_IN vs_in)
{
    Skin output = (Skin) 0;

    float4 pos = vs_in.pos;
    float3 normal = vs_in.normal.xyz;
	[unroll]
    for (int i = 0; i < 4; i++)
    {
        uint bone = vs_in.clusterIndex[i];
        float weight = vs_in.weights[i];
        matrix m = FetchBoneMatrix(bone);
        output.pos += weight * mul(pos, m);
        output.normal += weight * mul(normal, (float3x3) m);
    }
    return output;

}

PS_IN Main3D(VS_IN vs_in)
{
    PS_IN ps_out = (PS_IN) 0;
    Skin skinned = SkinVertex(vs_in);
	//skinned.pos.x *= -1;
	//skinned.normal.x *= -1;
    ps_out.pos = mul(skinned.pos, world);
    ps_out.eyeVector = cpos - ps_out.pos;
    ps_out.pos = mul(ps_out.pos, view);
    ps_out.pos = mul(ps_out.pos, projection);
    ps_out.normal = float4(skinned.normal, 0.0f);
    ps_out.normal = float4(mul((float3) ps_out.normal, (float3x3) world), 0.0f);
    ps_out.tex = vs_in.tex;
    ps_out.color = DirectionalLight(vs_in.normal, (float3) lightDirection, world);
    ps_out.color.a = 1.0f;
    return ps_out;
}

PS_IN Main3DNoSkin(VS_IN vs_in)
{
    PS_IN ps_out = (PS_IN) 0;
    ps_out.pos = mul(vs_in.pos, world);
    ps_out.eyeVector = cpos - ps_out.pos;
    ps_out.pos = mul(ps_out.pos, view);
    ps_out.pos = mul(ps_out.pos, projection);
    ps_out.normal = mul(float4(vs_in.normal.xyz, 0), world);
    ps_out.tex = vs_in.tex;
    ps_out.color = DirectionalLight(vs_in.normal, (float3) lightDirection, world);
    ps_out.color.a = 1.0f;
	//ノーマルマップ用
    float3 tangent = CalcTangent(vs_in.normal.xyz);
    float3 binormal = CalcBinormal(vs_in.normal.xyz);
    ps_out.tangentLightDirection = mul(lightDirection, InvTangentMatrix(tangent, binormal, vs_in.normal.xyz));
    return ps_out;
}
//////////////////////////////////////////////////////////////////////////////
//この先インスタンシング用
//////////////////////////////////////////////////////////////////////////////
//constant buffer ver
PS_IN Main3DInstancingNoSkin(VS_INSTANCING_IN vs_in)
{
    PS_IN ps_out = (PS_IN) 0;
    ps_out.pos = mul(vs_in.pos, vs_in.world);
    ps_out.eyeVector = cpos - ps_out.pos;
    ps_out.pos = mul(ps_out.pos, view);
    ps_out.pos = mul(ps_out.pos, projection);
    ps_out.normal = float4(mul((float3) vs_in.normal, (float3x3) vs_in.world), 0.0f);
    ps_out.tex = vs_in.tex;
    ps_out.color = DirectionalLight(vs_in.normal, (float3) lightDirection, world);
    ps_out.color.a = 1.0f;

    return ps_out;

}

Texture2D txAnimation : register(t3);
//texture ver
column_major float4x4 TextureFetchBoneMatrix(uint start, uint instanceID)
{
    column_major float4x4 pose;
    pose._11_21_31_41 = txAnimation.Load(int3(start + 0, instanceID, 0));
    pose._12_22_32_42 = txAnimation.Load(int3(start + 1, instanceID, 0));
    pose._13_23_33_43 = txAnimation.Load(int3(start + 2, instanceID, 0));
    pose._14_24_34_44 = txAnimation.Load(int3(start + 3, instanceID, 0));
    return pose;

}

Skin TextureSkinVertex(VS_INSTANCING_ANIMATION_IN vs_in)
{
    Skin output = (Skin) 0;

    float4 pos = vs_in.pos;
    float3 normal = vs_in.normal.xyz;
	[unroll]
    for (int i = 0; i < 4; i++)
    {
        uint bonePlace = vs_in.clusterIndex[i] * 4;
        float weight = vs_in.weights[i];
        column_major float4x4 m = TextureFetchBoneMatrix(bonePlace, vs_in.InstanceId);
        output.pos += weight * mul(pos, m);
        output.normal += weight * mul(normal, (float3x3) m);
    }
    return output;
}


PS_IN Main3DInstancing(VS_INSTANCING_ANIMATION_IN vs_in)
{
    PS_IN ps_out = (PS_IN) 0;
    Skin skinned = TextureSkinVertex(vs_in);
    ps_out.pos = mul(skinned.pos, vs_in.world);
    ps_out.eyeVector = cpos - ps_out.pos;
    ps_out.pos = mul(ps_out.pos, view);
    ps_out.pos = mul(ps_out.pos, projection);
    ps_out.normal = float4(mul((float3) vs_in.normal, (float3x3) vs_in.world), 0.0f);
    ps_out.tex = vs_in.tex;
    ps_out.color = DirectionalLight(vs_in.normal, (float3) lightDirection, world);
    ps_out.color.a = 1.0f;

    return ps_out;
}

PS_IN_POLYGON MainPolygon(VS_IN_POLYGON vs_in)
{
	PS_IN_POLYGON ps_out = (PS_IN_POLYGON)0;
	ps_out.pos = mul(vs_in.pos, world);
	ps_out.pos = mul(ps_out.pos, view);
	ps_out.pos = mul(ps_out.pos, projection);
	ps_out.tex = vs_in.tex;
	//float4 halfLambert = DirectionalLight(vs_in.normal, (float3) lightDirection, world)*0.5f + 0.5f;
	//halfLambert.a = 1.0f;
	ps_out.color = vs_in.color/**halfLambert*/;
	return ps_out;
}

