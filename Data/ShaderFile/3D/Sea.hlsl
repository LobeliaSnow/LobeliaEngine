//------------------------------------------------------------------------------------------------------
//
//		ヘッダー情報
//
//------------------------------------------------------------------------------------------------------
//C++側にも同じ定義がある
#include "../Define.h"

//FFT Ocean

cbuffer View : register(b0)
{
	column_major float4x4 view;
	column_major float4x4 previousView;
	column_major float4x4 projection;
	column_major float4x4 previousProjection;
	column_major float4x4 billboardMat;
	//ビュー+プロジェクション行列の逆行列
	column_major float4x4 inverseViewProjection;
	float4 cpos;
	struct Frustum
	{
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
//海面操作用定数バッファ 
cbuffer SeaInfo : register(b6) {
	float minDist : packoffset(c0.x);
	float maxDist : packoffset(c0.y);
	float maxDevide : packoffset(c0.z);
	float hegihtBias : packoffset(c0.w);
	float time : packoffset(c1.x);
	float refractiveRatio : packoffset(c1.y);
};
cbuffer CubeMap:register(b7) {
	column_major float4x4 views[6];
	column_major float4x4 projectionCube;
	int isLighting;
}


//色テクスチャ
Texture2D txDiffuse: register(t0);
//海面生成用テクスチャ 
Texture2D txDisplacementMap: register(t1);
//ノーマルマップ
Texture2D txNormalMap: register(t10);
#ifdef __PARABOLOID__
//双曲面環境マップ
Texture2DArray txParaboloid: register(t4);
#else
TextureCube txCube : register(t4);
#endif
//シーン情報
Texture2D txScene : register(t5);
//Texture2D txCaustics : register(t5);
SamplerState samLinear : register(s0);

//入力用構造体 
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

// 出力パッチ定数データ。
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
struct GS_OUT {
	float4 pos : SV_POSITION;
	float4 environmentPos : OLD_POSITION;
	float4 eyeVector : VECTOR0;
	float4 environmentEyeVector : VECTOR1;
	float4 normal : NORMAL0;
	float4 tangent : NORMAL1;
	float4 binormal : NORMAL2;
	float4 tangentLight : LIGHT3;
	float2 tex : TEXCOORD0;
	float2 screenPos : SCREEN_POS;
};
//単位行列
static const column_major float4x4 IDENTITY_MATRIX = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

//------------------------------------------------------------------------------------------------------
//
//		Common Function
//
//------------------------------------------------------------------------------------------------------
//法線の算出に使用
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
//ワールド空間から接空間へ変換するための
inline float4x4 InvTangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
	float4x4 mat =
	{
		{ float4(tangent, 0.0f) },
		{ float4(binormal, 0.0f) },
		{ float4(normal, 0.0f) },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	//正規直行系は転置するだけで逆行列となる
	return transpose(mat); // 転置
}
//接空間からワールド空間へ変換するための
inline float4x4 TangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
	float4x4 mat =
	{
		{ float4(tangent, 0.0f) },
		{ float4(binormal, 0.0f) },
		{ float4(normal, 0.0f) },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	return mat;
}

//------------------------------------------------------------------------------------------------------
//
//		Vertex Shader
//
//------------------------------------------------------------------------------------------------------
//環境マップ作製用
//環境マップ作成時はテッセレータ部分を使用しない
VS_OUT VS_CREATE_CUBE(VS_IN vs_in) {
	VS_OUT vs_out = (VS_OUT)0;
	//ワールド変換
	vs_out.pos = mul(vs_in.pos, world);
	vs_out.normal = mul(vs_in.normal, world);
	vs_out.tex = vs_in.tex;
	return vs_out;
}
//海用
//そのまま次のステージに流す 
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
// パッチ定数関数 
HS_TRI_OUT_DATA CalcTriConstants(InputPatch<VS_OUT, 3> ip, uint PatchID : SV_PrimitiveID) {
	HS_TRI_OUT_DATA Output;
	//距離に応じた分割数の算出
	float devide = 0.0f;
	//面の中心点算出
	float3 center = (ip[0].pos.xyz + ip[1].pos.xyz + ip[2].pos.xyz) / 3.0f;
	//中心点との距離
	float dist = length(cpos.xyz - center);
	//最小距離と最大距離の保証
	dist = clamp(dist, minDist, maxDist);
	//最小～最大距離での比率算出
	float ratio = (dist - minDist) / (maxDist - minDist);
	//最大分割数から上での比率を用い分割数を規定
	devide = (1.0f - ratio) * maxDevide + 1;
	//分割数の定義
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
	//テクスチャ座標算出
	output.tex = patch[0].tex * domain.x + patch[1].tex * domain.y + patch[2].tex * domain.z;
	//ディスプレースメント 
	float4 height = txDisplacementMap.SampleLevel(samLinear, output.tex, 0);

	//domainがUV値 それによりほか三点のコントロールポイントから座標を算出
	//位置算出
	float3 pos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;
	output.pos = float4(pos, 1.0f);
	//法線(実際に求めるのはGS)
	//ここで法線マップのテクセル入れてもいいかも？
	//ただ、法線マップのうまみが減ると思う
	output.normal = normalize(patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
	//output.normal.xyz = normalize(normal);
	//output.normal = normalize(mul(output.normal, world));
	//実際に頂点を動かす部分
	output.environmentPos = output.pos + output.normal * (sin(time + output.pos.x) / 3.0f);
	//ここを含めた高さを調節できるようにしたいのと、波の流れる向きを自由にする予定
	output.pos += output.normal * height;
	//波の合成
	float wave = sin(time + output.pos.x);
	wave += sin(time + output.pos.z)*0.5f;
	//wave += cos(time  * 0.5f - output.pos.x);
	//wave += sin(time  * 0.3f - output.pos.x);
	//wave += cos(time  * 0.1f - output.pos.x);
	wave /= 5.0f;
	output.pos += output.normal * wave;
	//output.tex.x = (sin(time + output.tex.x) / 3.14 + 1.0f)*0.5f;
	//ワールド変換等
	output.pos = mul(output.pos, world);
	output.environmentPos = mul(output.environmentPos, world);
	output.eyeVector = normalize(cpos - output.pos);
	output.environmentEyeVector = normalize(output.environmentPos - cpos);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);
	output.screenPos = (output.pos.xy / output.pos.w + 1.0f) * 0.5f;
	output.screenPos.y *= -1;
	return output;
}

//------------------------------------------------------------------------------------------------------
//
//		Geometry Shader
//
//------------------------------------------------------------------------------------------------------
//環境マップ作製用
[maxvertexcount(18)]
void GS_CREATE_CUBE(triangle VS_OUT gs_in[3], inout TriangleStream<GS_CREATE_CUBE_OUT> stream) {
	for (int i = 0; i < 6; i++) {
		GS_CREATE_CUBE_OUT ret = (GS_CREATE_CUBE_OUT)0;
		//レンダーターゲットの指定
		ret.renderTargetIndex = i;
		//スクリーンへ変換
		//[unroll]
		for (int j = 0; j < 3; j++) {
			ret.eyeVector = normalize(cpos - gs_in[j].pos);
			ret.pos = mul(gs_in[j].pos, views[i]);
			ret.pos = mul(ret.pos, projectionCube);
			ret.normal = gs_in[j].normal;
			ret.tex = gs_in[j].tex;
			//出力
			stream.Append(ret);
		}
		//命令の発行
		stream.RestartStrip();
	}
}
//海用
[maxvertexcount(3)]
void GS(triangle GS_OUT gs_in[3], inout TriangleStream<GS_OUT> stream) {
	GS_OUT gs_out[3] = (GS_OUT[3])0;
	//接空間算出 げーむつくろーから。理解は一旦後回し。
	//5次元から3次元に
	float3 cp0[3] = (float3[3])0;
	cp0[0] = float3(gs_in[0].environmentPos.x, gs_in[0].tex.x, gs_in[0].tex.y);
	cp0[1] = float3(gs_in[0].environmentPos.y, gs_in[0].tex.x, gs_in[0].tex.y);
	cp0[2] = float3(gs_in[0].environmentPos.z, gs_in[0].tex.x, gs_in[0].tex.y);
	float3 cp1[3] = (float3[3])0;
	cp1[0] = float3(gs_in[1].environmentPos.x, gs_in[1].tex.x, gs_in[1].tex.y);
	cp1[1] = float3(gs_in[1].environmentPos.y, gs_in[1].tex.x, gs_in[1].tex.y);
	cp1[2] = float3(gs_in[1].environmentPos.z, gs_in[1].tex.x, gs_in[1].tex.y);
	float3 cp2[3] = (float3[3])0;
	cp2[0] = float3(gs_in[2].environmentPos.x, gs_in[2].tex.x, gs_in[2].tex.y);
	cp2[1] = float3(gs_in[2].environmentPos.y, gs_in[2].tex.x, gs_in[2].tex.y);
	cp2[2] = float3(gs_in[2].environmentPos.z, gs_in[2].tex.x, gs_in[2].tex.y);
	//平面からＵＶ座標算出
	float u[3] = (float[3])0;
	float v[3] = (float[3])0;
	for (int i = 0; i < 3; i++) {
		float3 v0 = cp1[i] - cp0[i];
		float3 v1 = cp2[i] - cp1[i];
		float3 crossVector = cross(v0, v1);
		//縮退だけどどうしようもない
		//if (crossVector.x == 0.0f);
		u[i] = -crossVector.y / crossVector.x;
		v[i] = -crossVector.z / crossVector.x;
	}
	float4 tangent = normalize(float4(u[0], u[1], u[2], 0.0f));
	float4 binormal = normalize(float4(v[0], v[1], v[2], 0.0f));
	float4 normal = float4(normalize(cross(tangent, binormal)), 0.0f);
	//接空間へ変換するための行列
	const column_major float4x4 tangentMatrix = InvTangentMatrix(tangent, binormal, normal);
	float4 tangentLight = normalize(mul(lightDirection, tangentMatrix));
	for (int i = 0; i < 3; i++) {
		gs_out[i].pos = gs_in[i].pos;
		gs_out[i].normal.xyz = normal;
		gs_out[i].tangent.xyz = tangent;
		gs_out[i].binormal.xyz = binormal;
		gs_out[i].tangentLight = tangentLight;
		gs_out[i].eyeVector = normalize(mul(gs_in[i].eyeVector, tangentMatrix));
		//環境マップ用のカメラから位置へのベクトルを接空間へ変換
		gs_out[i].environmentEyeVector = normalize(mul(gs_in[i].environmentEyeVector, tangentMatrix));
		gs_out[i].tex = gs_in[i].tex;
		gs_out[i].screenPos = gs_in[i].screenPos;
		//ストリームに出力
		stream.Append(gs_out[i]);
	}
	stream.RestartStrip();
}

//------------------------------------------------------------------------------------------------------
//
//		Pixel Shader
//
//------------------------------------------------------------------------------------------------------
//環境マップ作製用
float4 PS_CREATE_CUBE(GS_CREATE_CUBE_OUT ps_in) :SV_Target{
	//色情報
	float4 diffuse = txDiffuse.Sample(samLinear, ps_in.tex);
	///全シェーダー共通の結果になるため、最適化が入るはず
	if (!isLighting) return diffuse;
	//ライティング
	float3 lambert = saturate(dot(ps_in.normal, lightDirection));
	//反射ベクトル取得
	float3 reflectValue = normalize(reflect(lightDirection, ps_in.normal));
	//スぺキュラ算出
	float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*1.0f;
	//最終結果
	return float4((diffuse.rgb*lambert + specular), diffuse.a);
}
//海用
float4 PS(GS_OUT ps_in) :SV_Target{
	//法線マップ読み込み
	float3 normalColor = txNormalMap.Sample(samLinear, ps_in.tex);
	float3 normalVector = normalize(2.0f * normalColor - 1.0f);
	//フレネル反射率計算
	float d = dot(-ps_in.environmentEyeVector.xyz, normalVector.xyz);
	float rt = sqrt(1.0f - refractiveRatio * refractiveRatio*(1.0f - d * d));
	float rs = (refractiveRatio*d - rt)*(refractiveRatio*d - rt) / ((refractiveRatio*d + rt)*(refractiveRatio*d + rt));
	float rp = (refractiveRatio*rt - d)*(refractiveRatio*rt - d) / ((refractiveRatio*rt + d)*(refractiveRatio*rt + d));
	float alpha = (rs + rp) / 2.0f;
	alpha = saturate(alpha);
#ifdef __PARABOLOID__//双曲面環境マップ
	float3 ref = reflect(ps_in.environmentEyeVector.xyz, normalVector.xyz);
	//接空間からワールド空間へ変換用
	const column_major float4x4 tangentMatrix = TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal);
	ref = mul(ref, tangentMatrix);
	float2 paraboloidTex0; float2 paraboloidTex1;
	paraboloidTex0.x = 0.5f*(1 + ref.x / (1 + ref.z));
	paraboloidTex0.y = 0.5f*(1 - ref.y / (1 + ref.z));
	paraboloidTex1.x = 0.5f*(1 + ref.x / (1 - ref.z));
	paraboloidTex1.y = 0.5f*(1 - ref.y / (1 - ref.z));
	bool isFront = ref.z + 0.5f;
	//環境マップ
	float4 front = txParaboloid.Sample(samLinear, float3(paraboloidTex0,0));
	float4 back = txParaboloid.Sample(samLinear, float3(paraboloidTex1,1));
	float4 reflectEnvironment;
	if (isFront > 0.5f) reflectEnvironment = front;
	else reflectEnvironment = back;
	return float4(reflectEnvironment.rgb, alpha);
#else//この先キューブマップ
	//環境マップ読み込み
	float3 reflectRay = normalize(reflect(ps_in.environmentEyeVector.xyz, normalVector));
	const float eta = 0.67;  // 屈折率の比
	const float f = (1.0 - eta) * (1.0 - eta) / ((1.0 + eta) * (1.0 + eta));
	float3 refractRay = normalize(refract(ps_in.environmentEyeVector.xyz, normalVector, eta));
	//接空間からワールド空間へ変換用
	const column_major float4x4 tangentMatrix = TangentMatrix(ps_in.tangent, ps_in.binormal, ps_in.normal);
	//ここではまだ接空間なのでワールド空間に戻す(苦肉の策)
	reflectRay = mul(reflectRay, tangentMatrix);
	refractRay = normalize(mul(refractRay, tangentMatrix));
	//ここでの反射ベクトルは、ワールド空間のものを要求
	float4 reflectEnvironment = txCube.Sample(samLinear, reflectRay);
	//Zによって歪み率を変えてやることで破綻を最小限に抑える
	//↑により一部視線からの屈折がおかしいので見直すこと
	float2 refractPoint = ps_in.screenPos + refractRay.xy * 0.08f * max(refractRay.z,0.3f);
	if (refractPoint.x > 1.0f || refractPoint.x < 0.0f)refractPoint.x = ps_in.screenPos.x;
	if (refractPoint.y > 1.0f || refractPoint.y < 0.0f)refractPoint.y = ps_in.screenPos.y;
	float4 refractColor = txScene.Sample(samLinear, refractPoint);
	return float4(saturate(reflectEnvironment.rgb*alpha + refractColor.rgb * (1.0f - alpha)),1.0f);
	//float ratio = f + (1.0f - f)*pow(1.0f - dot(-ps_in.environmentEyeVector.xyz, normalVector), 5.0f);
	//return float4(refractEnvironment.rgb, 1.0f);
	//return float4(refractEnvironment.rgb, ratio);
	//return float4( + refractColor * (1.0f - alpha),1.0f);
	//float4 caustics = txCaustics.Sample(samLinear, ps_in.tex*6.0f)*0.3f;
	//return float4(reflectEnvironment.rgb + caustics.rgb, alpha);
	//return float4(lerp(reflectEnvironment,refractEnvironment, ratio).rgb,1.0f);
	////ライティング
	//float3 lambert = saturate(dot(normalVector, ps_in.tangentLight));
	////反射ベクトル取得
	//float3 reflectValue = normalize(reflect(ps_in.tangentLight, normalVector));
	////スぺキュラ算出
	//float3 specular = pow(saturate(dot(reflectValue, ps_in.eyeVector.xyz)), 2)*3.0f;
	//return float4(environment.rgb, 1.0f);
#endif
}
