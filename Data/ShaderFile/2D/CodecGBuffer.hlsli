//----------------------------------------------------------------------------------------------------------------------
//
//		関数軍
//
//----------------------------------------------------------------------------------------------------------------------
uint4 LoadUintTexture(in Texture2D<uint4> tex, in float2 uv) {
	//圧縮された情報の読み込み
	float2	pixelSize = (float2)0.0f;
	tex.GetDimensions(pixelSize.x, pixelSize.y);
	return tex.Load(uint3(uv*pixelSize, 0));
}
//0~255で表すことが可能な色データ(SDR)の圧縮
//HDRデータの圧縮はできません
uint EncodeSDRColor(in float4 color) {
	//0.0f~1.0fの間へ丸める
	color = saturate(color);
	//0~255へ
	uint4 colorUint = (uint4)(color * 255.0f);
	//ビット圧縮
	return uint(colorUint.r | colorUint.g << 8 | colorUint.b << 16 | colorUint.a << 24);
}
//圧縮されたSDRデータの解凍
float4 DecodeSDRColor(in uint encode_color) {
	//ビット列を分離
	uint4 colorUint = uint4(encode_color.r & 255, (encode_color.r >> 8) & 255, (encode_color.r >> 16) & 255, encode_color.r >> 24);
	//0.0f~1.0fへ
	return (float4)colorUint / 255.0f;
}
//正規化された法線の圧縮
uint EncodeNormalVector(in float3 normal) {
	//0.0~1.0へ
	normal = normal * 0.5f + 0.5;
	//xy情報からzを再構築するので、zをそぎ落とし、
	//floatをhalfにまで情報を落とし、パッキング、uintとしてビット列を認識させる
	return asuint(f32tof16(normal.x) | f32tof16(normal.y) << 16);
}
//圧縮された法線のデコード
float4 DecodeNormalVector(in uint encode_normal) {
	//法線のデコード Killzone2方式採用
	//http://aras-p.info/texts/CompactNormalStorage.html#method01xy
	float4 normal = (float4)0.0f;
	//要素を展開し、バイト列をfloatとして認識
	normal.x = asfloat(f16tof32(encode_normal));
	normal.y = asfloat(f16tof32(encode_normal >> 16));
	normal.xy = normal.xy * 2.0 - 1.0;
	//三平方の定理による法線Zの復元 メモ帳参照 Killzone2でも使用
	//1.0fは1.0fの2乗を省略して書いていて、dot(v,v)は長さの二乗なので三平方
	normal.z = sqrt(1.0f - dot(normal.xy, normal.xy));
	return normal;
}
//深度とUVからViewProjectionPosを復元
float4 DecodeDepthToVPPos(in float depth, in float2 uv) {
	return float4(uv.x*2.0f - 1.0f, (1.0f - uv.y)*2.0f - 1.0f, depth, 1.0f);
}
//ViewProjectionPosからWorldPosを復元
float4 DecodeVPPosToWorldPos(in float4 vp_pos, in column_major float4x4 inverse_vp) {
	float4 worldPos = mul(vp_pos, inverse_vp);
	return worldPos / worldPos.w;
}
//深度値等からワールド座標の復元
float4 DecodeDepthToWorldPos(in float depth, in float2 uv, in column_major float4x4 inverse_view_proj) {
	float4 vpPos = DecodeDepthToVPPos(depth, uv);
	return DecodeVPPosToWorldPos(vpPos, inverse_view_proj);
}
//NDC空間へ正規化されたDepthが対象
uint EncodeDepth(in float depth) { return asuint(depth); }
float DecodeDepth(in uint encode_depth) { return asfloat(encode_depth); }
