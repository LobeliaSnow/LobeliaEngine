cbuffer View : register(b0)
{
	column_major float4x4 view;
	column_major float4x4 projection;
	column_major float4x4 billboardMat;
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
};

cbuffer Material : register(b2)
{
	float4 diffuse;
	float4 ambient;
	float4 specular;
	float4 texColor;
}

cbuffer Bone : register(b3)
{
	//1mesh辺り255本のボーン
	column_major float4x4 keyFrames[256];
}

cbuffer Environment : register(b4)
{
	//正規化されています
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
	//サーフェスのサイズ
	float width : packoffset(c0.x);
	float height : packoffset(c0.y);
}

//0~9までファイルから読み込んだテクスチャ情報
Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txSpecular : register(t2);

//10~レンダーターゲットからの入力
Texture2D rtDiffuse : register(t10);
Texture2D rtSpecular : register(t11);

//20~インスタンシングレンダラ用
Texture2D txDiffuse0 : register(t20);
Texture2D txDiffuse1 : register(t21);
Texture2D txDiffuse2 : register(t22);
Texture2D txDiffuse3 : register(t23);
Texture2D txDiffuse4 : register(t24);
Texture2D txDiffuse5 : register(t25);
Texture2D txDiffuse6 : register(t26);
Texture2D txDiffuse7 : register(t27);


SamplerState samLinear : register(s0);

