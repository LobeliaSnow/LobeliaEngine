//----------------------------------------------------------------------------------------------------------------------
//
//		�֐��R
//
//----------------------------------------------------------------------------------------------------------------------
uint4 LoadUintTexture(in Texture2D<uint4> tex, in float2 uv) {
	//���k���ꂽ���̓ǂݍ���
	float2	pixelSize = (float2)0.0f;
	tex.GetDimensions(pixelSize.x, pixelSize.y);
	return tex.Load(uint3(uv*pixelSize, 0));
}
//0~255�ŕ\�����Ƃ��\�ȐF�f�[�^(SDR)�̈��k
//HDR�f�[�^�̈��k�͂ł��܂���
uint EncodeSDRColor(in float4 color) {
	//0.0f~1.0f�̊Ԃ֊ۂ߂�
	color = saturate(color);
	//0~255��
	uint4 colorUint = (uint4)(color * 255.0f);
	//�r�b�g���k
	return uint(colorUint.r | colorUint.g << 8 | colorUint.b << 16 | colorUint.a << 24);
}
//���k���ꂽSDR�f�[�^�̉�
float4 DecodeSDRColor(in uint encode_color) {
	//�r�b�g��𕪗�
	uint4 colorUint = uint4(encode_color.r & 255, (encode_color.r >> 8) & 255, (encode_color.r >> 16) & 255, encode_color.r >> 24);
	//0.0f~1.0f��
	return (float4)colorUint / 255.0f;
}
//���K�����ꂽ�@���̈��k
uint EncodeNormalVector(in float3 normal) {
	//0.0~1.0��
	normal = normal * 0.5f + 0.5;
	//xy��񂩂�z���č\�z����̂ŁAz���������Ƃ��A
	//float��half�ɂ܂ŏ��𗎂Ƃ��A�p�b�L���O�Auint�Ƃ��ăr�b�g���F��������
	return asuint(f32tof16(normal.x) | f32tof16(normal.y) << 16);
}
//���k���ꂽ�@���̃f�R�[�h
float4 DecodeNormalVector(in uint encode_normal) {
	//�@���̃f�R�[�h Killzone2�����̗p
	//http://aras-p.info/texts/CompactNormalStorage.html#method01xy
	float4 normal = (float4)0.0f;
	//�v�f��W�J���A�o�C�g���float�Ƃ��ĔF��
	normal.x = asfloat(f16tof32(encode_normal));
	normal.y = asfloat(f16tof32(encode_normal >> 16));
	normal.xy = normal.xy * 2.0 - 1.0;
	//�O�����̒藝�ɂ��@��Z�̕��� �������Q�� Killzone2�ł��g�p
	//1.0f��1.0f��2����ȗ����ď����Ă��āAdot(v,v)�͒����̓��Ȃ̂ŎO����
	normal.z = sqrt(1.0f - dot(normal.xy, normal.xy));
	return normal;
}
//�[�x��UV����ViewProjectionPos�𕜌�
float4 DecodeDepthToVPPos(in float depth, in float2 uv) {
	return float4(uv.x*2.0f - 1.0f, (1.0f - uv.y)*2.0f - 1.0f, depth, 1.0f);
}
//ViewProjectionPos����WorldPos�𕜌�
float4 DecodeVPPosToWorldPos(in float4 vp_pos, in column_major float4x4 inverse_vp) {
	float4 worldPos = mul(vp_pos, inverse_vp);
	return worldPos / worldPos.w;
}
//�[�x�l�����烏�[���h���W�̕���
float4 DecodeDepthToWorldPos(in float depth, in float2 uv, in column_major float4x4 inverse_view_proj) {
	float4 vpPos = DecodeDepthToVPPos(depth, uv);
	return DecodeVPPosToWorldPos(vpPos, inverse_view_proj);
}
//NDC��Ԃ֐��K�����ꂽDepth���Ώ�
uint EncodeDepth(in float depth) { return asuint(depth); }
float DecodeDepth(in uint encode_depth) { return asfloat(encode_depth); }
