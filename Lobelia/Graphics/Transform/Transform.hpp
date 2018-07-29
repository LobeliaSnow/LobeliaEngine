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
	//これはModelクラスの行列部分だけコピペしてきたもの
	//そのうち統一化を図りたい
	class Transformer {
	public:
		Transformer();
		//移動 行列計算も行われます
		void Translation(const Math::Vector3& pos);
		void Translation(float x, float y, float z);
		void TranslationMove(const Math::Vector3& move);
		void TranslationMove(float x, float y, float z);
		//回転 行列計算も行われます
		void RotationQuaternion(const DirectX::XMVECTOR& quaternion);
		void RotationAxis(const Math::Vector3& axis, float rad);
		void RotationRollPitchYow(const Math::Vector3& rpy);
		void RotationYAxis(float rad);
		//拡縮 行列計算も行われます
		void Scalling(const Math::Vector3& scale);
		void Scalling(float x, float y, float z);
		void Scalling(float scale);
		//更新処理
		void CalcWorldMatrix();
		//行列取得
		void GetTranslateMatrix(DirectX::XMMATRIX* translate);
		void CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate);
		void GetScallingMatrix(DirectX::XMMATRIX* scalling);
		void CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling);
		void GetRotationMatrix(DirectX::XMMATRIX* rotation);
		void CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation);
		//転置行列を返します
		void GetWorldMatrix(DirectX::XMMATRIX* t_world);
		void GetWorldMatrixTranspose(DirectX::XMMATRIX* world);
		void CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world);
		const Transform3D& GetTransform();
	private:
		//transformから移動行列生成
		void CalcTranslateMatrix();
		//transformから拡縮行列生成
		void CalcScallingMatrix();
	private:
		Transform3D transform;
		//そのうちXMFLOAT4X4に変換
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX translate;
		DirectX::XMMATRIX scalling;
		DirectX::XMMATRIX rotation;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

}