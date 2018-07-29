#pragma once
namespace Lobelia::Graphics {
	struct Transform2D {
		Math::Vector2 position;
		Math::Vector2 scale;
		float rotation;
	};
	struct Transform3D {
		Math::Vector3 position;
		Math::Vector3 scale;
		Math::Vector3 rotation;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//	Transformer
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//�����Model�N���X�̍s�񕔕������R�s�y���Ă�������
	//���̂������ꉻ��}�肽��
	class Transformer {
	public:
		Transformer();
		//�ړ� �s��v�Z���s���܂�
		void Translation(const Math::Vector3& pos);
		void Translation(float x, float y, float z);
		void TranslationMove(const Math::Vector3& move);
		void TranslationMove(float x, float y, float z);
		//��] �s��v�Z���s���܂�
		void RotationQuaternion(const DirectX::XMVECTOR& quaternion);
		void RotationAxis(const Math::Vector3& axis, float rad);
		void RotationRollPitchYow(const Math::Vector3& rpy);
		void RotationYAxis(float rad);
		//�g�k �s��v�Z���s���܂�
		void Scalling(const Math::Vector3& scale);
		void Scalling(float x, float y, float z);
		void Scalling(float scale);
		//�X�V����
		void CalcWorldMatrix();
		//�s��擾
		void GetTranslateMatrix(DirectX::XMMATRIX* translate);
		void CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate);
		void GetScallingMatrix(DirectX::XMMATRIX* scalling);
		void CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling);
		void GetRotationMatrix(DirectX::XMMATRIX* rotation);
		void CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation);
		//�]�u�s���Ԃ��܂�
		void GetWorldMatrix(DirectX::XMMATRIX* t_world);
		void GetWorldMatrixTranspose(DirectX::XMMATRIX* world);
		void CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world);
		const Transform3D& GetTransform();
	private:
		//transform����ړ��s�񐶐�
		void CalcTranslateMatrix();
		//transform����g�k�s�񐶐�
		void CalcScallingMatrix();
	private:
		Transform3D transform;
		//���̂���XMFLOAT4X4�ɕϊ�
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX translate;
		DirectX::XMMATRIX scalling;
		DirectX::XMMATRIX rotation;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

}