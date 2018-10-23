//そのうちバイトニックソートも頑張って汎用化したいけど思いつかない。。。。


//定数バッファ
cbuffer Raycaster :register(b11) {
	float4 rayBegin;
	float4 rayEnd;
	column_major float4x4 world;
	column_major float4x4 worldInverse;
};

//StructuredBuffer用
struct RayInput {
	float3 pos[3];
};
//RWStructuredBuffer用
struct RayOutput {
	int hit;
	float length;
	float3 normal;
};

//入力用バッファ
StructuredBuffer<RayInput> rayInput :register(t0);
//出力用バッファ
RWStructuredBuffer<RayOutput> rayOutput :register(u0);

//平面方程式を計算する
//参考
//http://www.sousakuba.com/Programming/gs_plane.html
float4 ComputePlaneEquation(float3 v0, float3 v1, float3 v2) {
	//平面方程式
	//ax + by + cz + d = 0で表す
	//平面の法線が(a, b, c)であることと、法線方向へd進んだ先に平面があることを示す
	//距離は法線と頂点一つで計算可能
	//辺を算出
	float3 edge0, edge1;
	edge0 = v1 - v0;
	edge1 = v2 - v1;
	float4 plane;
	//法線を算出
	float3 normal = cross(edge0, edge1);
	plane.xyz = normal;
	//距離を算出
	plane.w = -dot(v0.xyz, plane.xyz);
	return plane;
}

//点が三角形の内部にある場合true、なければfalse
bool IsInside(float3 pos, float3 v0, float3 v1, float3 v2) {
	//辺算出
	float3 edge0, edge1, edge2;
	edge0 = v1 - v0; edge1 = v2 - v1; edge2 = v0 - v2;
	//平面方程式より法線取得
	float4 plane = ComputePlaneEquation(v0, v1, v2);
	float3 normal = normalize(plane.xyz);
	//交点に向かうベクトルを算出
	float3 v = pos - v0;
	float3 cross0 = cross(edge0, v);
	v = pos - v1;
	float3 cross1 = cross(edge1, v);
	v = pos - v2;
	float3 cross2 = cross(edge2, v);
	//内積の計算
	float dot0 = dot(normal, cross0);
	float dot1 = dot(normal, cross1);
	float dot2 = dot(normal, cross2);
	//交点判定
	return (dot0 >= 0 && dot1 >= 0 && dot2 >= 0);
}

//平面と交差していれば距離、していなければ0を返す
//参考
//http://www.hiramine.com/programming/graphics/3d_planesegmentintersection.html
//NorthBrain書籍
float4 Intersect(float4 plane, float3 ray_begin, float3 ray_end, float3 v0, float3 v1, float3 v2) {
	//平面との交点を算出
	//plane * ray_end
	float4 prb = float4(plane.xyz * ray_begin, plane.w);
	float4 pre = float4(plane.xyz * ray_end, plane.w);
	float prba = prb.x + prb.y + prb.z;
	float prea = pre.x + pre.y + pre.z;
	float t = -(prea + pre.w) / (prba - prea);
	if (t >= 0.0f && t <= 1.0f) {
		//交点座標を得る あくまでも無限平面相手なので、内点判定が必要
		//tが求まっていれば両端の点から求まる
		float3 pos = ray_begin * t + ray_end * (1.0f - t);
		//その交点の内点判定
		if (IsInside(pos, v0, v1, v2)) {
			//当たった場所を返す
			return float4(pos, 1.0f);
		}
		//nanに飛ばす
		return (float4)0.0f / 0.0f;
	}
	//nanに飛ばす
	return (float4)0.0f / 0.0f;
}

//スレッド数はアプリ側で指定する
[numthreads(1, 1, 1)]
void RaycastCS(uint3 id : SV_DispatchThreadID) {
	float3 v0 = rayInput[id.x].pos[0];
	float3 v1 = rayInput[id.x].pos[1];
	float3 v2 = rayInput[id.x].pos[2];
	//ローカル空間へRayを引き込む
	float3 begin = mul(rayBegin, worldInverse).xyz;
	float3 end = mul(rayEnd, worldInverse).xyz;
	//平面方程式を算出
	float4 plane = ComputePlaneEquation(v0, v1, v2);
	//交点算出し、交点距離を求める
	float4 pos = Intersect(plane, begin, end, v0, v1, v2);
	//ローカルでの計算からワールドへもっていく
	pos = mul(pos, world);
	rayOutput[id.x].hit = !isnan(pos);
	//当たった長さ算出
	float dist = length(pos - rayBegin);
	rayOutput[id.x].length = dist;
	rayOutput[id.x].normal = normalize(plane.xyz);
}
