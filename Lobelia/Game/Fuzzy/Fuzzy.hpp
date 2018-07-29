#pragma once
//�t�@�W�[���_���������邽�߂̃w���p�[�Q
namespace Lobelia::Game {
	//�t�@�W�[�菇
	//1,�}�b�s���O�v���Z�X(�t�@�W�[�� �N���X�v�W���������o�[�V�b�v�֐��ɂ�郁���o�[�V�b�v�x�Z�o)
	//2,���[���Ɋ�Â��t�@�W�[�o��
	//3,���̏o�͂��N���X�v��(��t�@�W�[��)
	//�N���X�v���͎����Ŏ������邵���Ȃ�
	//Lobelia::Math�ɉϒ��łƂ��Min�֐���Max�֐�������̂ŃN���X�v���̍ۂɖ𗧂ĂĂق���

	//�����o�[�V�b�v�֐�
	//���͂����N���X�v�l���烁���o�[�V�b�v�ǎZ�o���邽�߂̂���
	class FuzzyMembership {
	public:
		//�E���オ��
		static float Grade(float value, float min, float max);
		//�E��������
		static float ReverseGrade(float value, float min, float max);
		//�O�p�`�^
		static float Triangle(float value, float min, float center, float max);
		//��`�^
		static float Trapezoid(float value, float min, float center_left, float center_right, float max);
	private:
		//���`�X�΂���邽�߂̃w���p�[�֐�
		static float CompareGrade(float value, float min, float max) { return (value / (max - min)) - (min / (max - min)); }
		static float CompareReverseGrade(float value, float min, float max) { return (max / (max - min)) - (value / (max - min)); }
	};
	//�����o�[�V�b�v�x�𒲐����邽�߂̂���
	class FuzzyHedges {
	public:
		//�����̓��
		static float Very(float value) { return value*value; }
		//�����̕�����
		static float NotVery(float value) { return sqrtf(value); }
	};
	//�t�@�W�[���[��
	class FuzzyRule {
		//�_���� &&
		static float And(float value0, float value1) { return fminf(value0, value1); }
		//�_���a ||
		static float Or(float value0, float value1) { return fmaxf(value0, value1); }
		//�_���ے� !
		static float Not(float value) { return 1.0f - value; }
	};
}