//カメラ情報 
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


//ワールド変換行列
cbuffer World : register(b1) {
	column_major float4x4 world;
};
//環境用
cbuffer Environment : register(b4)
{
	//正規化されています
	float4 lightDirection;
	float4 ambientColor;
	float4 fogInfo;
}

cbuffer EnvironmentInfo : register(b7) {
	//双曲面なので二つ
	column_major float4x4 views[2];
	column_major float4x4 projectionParaboloid;
	float zNear;
	float zFar;
};
cbuffer DebugInfo : register(b8) {
	//双曲面なので二つ
	int textureIndex : packoffset(c0.x);
};
//色テクスチャ
Texture2D txDiffuse: register(t0);
//双曲面環境マップ
Texture2DArray txParaboloid: register(t4);
SamplerState samLinear : register(s0);

//入力用構造体 
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
	//ワンパスパラボロイド用
	//レンダーターゲットを指定
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};
//デバッグ用
struct VS_IN_TEX
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};

//デバッグ用スプライト構造体
struct PS_IN_TEX
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};

//引数の位置はワールド変換済みのものを指す
float4 Paraboloid(float4 position, int render_id) {
	float4 ret = (float4)0;
	float4 pos = mul(position, views[render_id]);
	//長さ取得
	float l = length(pos);
	//ちょっと何やってるかわからん。
	//後で考える
	ret.xy = pos.xy*l*(l - pos.z) / dot(pos.xy, pos.xy);
	ret.z = (l - zNear)*zFar / (zFar - zNear);
	//苦渋のif 回避する方法がないか考える
	if (ret.z >= 0.0f) {//描画対象が自分の表面だった場合
		ret.z = (l - zNear)*zFar / (zFar - zNear);
		ret.w = l;
	}
	else {//描画対象が自分の裏面だった場合
		ret.z = 0.0f;
		//無限遠方へ
		ret.w = -0.00001f;
	}
	return ret;
}

//環境マップ作製用
VS_CREATE_PARABOLOID_OUT VS_CREATE_PARABOLOID(VS_IN vs_in) {
	VS_CREATE_PARABOLOID_OUT vs_out = (VS_CREATE_PARABOLOID_OUT)0;
	vs_out.pos = mul(vs_in.pos, world);
	vs_out.eyeVector = normalize(cpos - vs_out.pos);
	vs_out.normal = mul(vs_in.normal, world);
	vs_out.tex = vs_in.tex;
	//vs_out.renderTargteIndex = 0;
	return vs_out;
}

//デバッグ用
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
			//出力
			stream.Append(gs_out);
		}
		//命令の発行
		stream.RestartStrip();
	}
}

//環境マップ作製用
float4 PS_CREATE_PARABOLOID(GS_CREATE_PARABOLOID_OUT ps_in) :SV_Target{
	//色情報
	float4 diffuse = txDiffuse.Sample(samLinear, ps_in.tex);
	return float4(diffuse.rgb, diffuse.a);
	//ライティング
	float3 lambert = saturate(dot(ps_in.normal, lightDirection));
	//反射ベクトル取得
	float3 reflectValue = normalize(reflect(lightDirection, ps_in.normal));
	//スぺキュラ算出
	float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	return float4((diffuse.rgb*lambert + specular), diffuse.a);
}
//デバッグ用スプライト表示
float4 PS_DEBUG_SPRITE(PS_IN_TEX ps_in) :SV_Target{
	return float4(txParaboloid.Sample(samLinear, float3(ps_in.tex,textureIndex)).rgb,1.0f);
}