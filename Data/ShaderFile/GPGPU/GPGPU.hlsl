//���̂����o�C�g�j�b�N�\�[�g���撣���Ĕėp�����������ǎv�����Ȃ��B�B�B�B

//���󗠖ʂł�������Ǝv���̂ł��̎���C��

//�萔�o�b�t�@
cbuffer Raycaster :register(b11) {
	float4 rayBegin;
	float4 rayEnd;
	column_major float4x4 world;
	column_major float4x4 worldInverse;
};

//StructuredBuffer�p
struct RayInput {
	float3 pos[3];
};
//RWStructuredBuffer�p
struct RayOutput {
	int hit;
	float length;
	float3 normal;
};

//���͗p�o�b�t�@
StructuredBuffer<RayInput> rayInput :register(t1);
//�o�͗p�o�b�t�@
RWStructuredBuffer<RayOutput> rayOutput :register(u1);

//���ʕ��������v�Z����
//�Q�l
//http://www.sousakuba.com/Programming/gs_plane.html
float4 ComputePlaneEquation(float3 v0, float3 v1, float3 v2) {
	//���ʕ�����
	//ax + by + cz + d = 0�ŕ\��
	//���ʂ̖@����(a, b, c)�ł��邱�ƂƁA�@��������d�i�񂾐�ɕ��ʂ����邱�Ƃ�����
	//�����͖@���ƒ��_��Ōv�Z�\
	//�ӂ��Z�o
	float3 edge0, edge1;
	edge0 = v1 - v0;
	edge1 = v2 - v1;
	float4 plane;
	//�@�����Z�o
	float3 normal = cross(edge0, edge1);
	plane.xyz = normal;
	//�������Z�o
	plane.w = -dot(v0.xyz, plane.xyz);
	return plane;
}

//�_���O�p�`�̓����ɂ���ꍇtrue�A�Ȃ����false
bool IsInside(float3 pos, float3 v0, float3 v1, float3 v2) {
	//�ӎZ�o
	float3 edge0, edge1, edge2;
	edge0 = v1 - v0; edge1 = v2 - v1; edge2 = v0 - v2;
	//���ʕ��������@���擾
	float4 plane = ComputePlaneEquation(v0, v1, v2);
	float3 normal = normalize(plane.xyz);
	//��_�Ɍ������x�N�g�����Z�o
	float3 v = pos - v0;
	float3 cross0 = cross(edge0, v);
	v = pos - v1;
	float3 cross1 = cross(edge1, v);
	v = pos - v2;
	float3 cross2 = cross(edge2, v);
	//���ς̌v�Z
	float dot0 = dot(normal, cross0);
	float dot1 = dot(normal, cross1);
	float dot2 = dot(normal, cross2);
	//��_����
	return (dot0 >= 0 && dot1 >= 0 && dot2 >= 0);
}

//���ʂƌ������Ă���΋����A���Ă��Ȃ����0��Ԃ�
//�Q�l
//http://www.hiramine.com/programming/graphics/3d_planesegmentintersection.html
//NorthBrain����
float4 Intersect(float4 plane, float3 ray_begin, float3 ray_end, float3 v0, float3 v1, float3 v2) {
	//���ʂƂ̌�_���Z�o
	//plane * ray_end
	float4 prb = float4(plane.xyz * ray_begin, plane.w);
	float4 pre = float4(plane.xyz * ray_end, plane.w);
	float prba = prb.x + prb.y + prb.z;
	float prea = pre.x + pre.y + pre.z;
	float t = -(prea + pre.w) / (prba - prea);
	if (t >= 0.0f && t <= 1.0f) {
		//��_���W�𓾂� �����܂ł��������ʑ���Ȃ̂ŁA���_���肪�K�v
		//t�����܂��Ă���Η��[�̓_���狁�܂�
		float3 pos = ray_begin * t + ray_end * (1.0f - t);
		//���̌�_�̓��_����
		if (IsInside(pos, v0, v1, v2)) {
			//���������ꏊ��Ԃ�
			return float4(pos, 1.0f);
		}
		//nan�ɔ�΂�
		return (float4)0.0f / 0.0f;
	}
	//nan�ɔ�΂�
	return (float4)0.0f / 0.0f;
}

//�X���b�h���̓A�v�����Ŏw�肷��
[numthreads(1, 1, 1)]
void RaycastCS(uint3 id : SV_DispatchThreadID) {
	float3 v0 = rayInput[id.x].pos[0];
	float3 v1 = rayInput[id.x].pos[1];
	float3 v2 = rayInput[id.x].pos[2];
	//���[�J����Ԃ�Ray����������
	float3 begin = mul(rayBegin, worldInverse).xyz;
	float3 end = mul(rayEnd, worldInverse).xyz;
	//���ʕ��������Z�o
	float4 plane = ComputePlaneEquation(v0, v1, v2);
	//��_�Z�o���A��_���������߂�
	float4 pos = Intersect(plane, begin, end, v0, v1, v2);
	//���[�J���ł̌v�Z���烏�[���h�ւ����Ă���
	pos = mul(pos, world);
	rayOutput[id.x].hit = !isnan(pos);
	//�������������Z�o
	float dist = length(pos - rayBegin);
	rayOutput[id.x].length = dist;
	rayOutput[id.x].normal = normalize(plane.xyz);
}
