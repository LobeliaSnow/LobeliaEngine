struct VS_IN_TEX
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 tex : TEXCOORD0;
};
struct VS_IN_INSTANCING_TEX
{
    float4 pos : POSITION0;
    float2 tex : TEXCOORD0;
    column_major float4x4 ndcTrans : NDC_I;
    float4 col : COLOR0_I;
    float4 uvTrans : UV_I;
    uint InstanceId : SV_InstanceID; // インスタンスＩＤ};
};
struct PS_IN_TEX
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 tex : TEXCOORD0;
};

struct VS_IN_INSTANCING_TEX_RENDERER
{
    float4 pos : POSITION0;
    float2 tex : TEXCOORD0;
    column_major float4x4 ndcTrans : NDC_I;
    float4 col : COLOR0_I;
    float4 uvTrans : UV_I;
    int textureSlot : TEX_SLOT_I;
    uint InstanceId : SV_InstanceID; // インスタンスＩＤ};
};
struct PS_IN_TEX_RENDERER
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    int textureSlot : TEX_SLOT;
	float2 tex : TEXCOORD0;
};
