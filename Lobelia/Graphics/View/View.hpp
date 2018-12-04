#pragma once
namespace Lobelia::Graphics {
	class View {
	private:
		struct Frustum {
			Math::Vector4 center[6];
			Math::Vector4 normal[6];
		};
		//TODO : XMFLOAT4X4�ɕς���
		struct Constant {
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX previousView;
			DirectX::XMMATRIX projection;
			DirectX::XMMATRIX previousProjection;
			DirectX::XMMATRIX billboardMat;
			DirectX::XMMATRIX inverseViewProjection;
			Math::Vector4 pos;
			Frustum frustum;
		};
		struct Data {
			DirectX::XMVECTOR eye;
			DirectX::XMVECTOR at;
			DirectX::XMVECTOR up;
		};
	private:
		static DirectX::XMMATRIX nowView;
		static DirectX::XMMATRIX nowProjection;
	public:
		//private�ɂ��ăQ�b�^�[���
		static Math::Vector2 nowSize;
	private:
		D3D11_VIEWPORT viewport = {};
		std::unique_ptr<ConstantBuffer<Constant>> constantBuffer;
		Constant buffer;
		Data data;
		Math::Vector2 size;
		float nearZ;
		float farZ;
		float fov;
		float aspect;
		Frustum frustum;
	private:
		void CreateProjection(float fov_rad, float aspect, float near_z, float far_z);
		void CreateView(const Data& data);
		void CreateBillboardMat(const Data& data);
		void CreateViewport(const Math::Vector2& pos, const Math::Vector2& size);
	public:
		View(const Math::Vector2& left_up, const Math::Vector2& size, float fov_rad = PI / 4.0f, float near_z = 1.0f, float far_z = 1000.0f);
		virtual ~View();
		void SetEyePos(const Math::Vector3& pos);
		void SetEyeTarget(const Math::Vector3& target);
		void SetEyeUpDirection(const Math::Vector3& up_direction);
		Math::Vector3 GetEyePos();
		Math::Vector3 GetEyeTarget();
		Math::Vector3 GetEyeUpDirection();
		void SetFov(float fov_rad);
		void SetNear(float near_z);
		void SetFar(float far_z);
		float GetNear();
		float GetFar();
		float GetAspect();
		float GetFov();
		void ChangeViewport(const Math::Vector2& pos, const Math::Vector2& size);
		void CreateFrustum();
		bool IsFrustumRange(const Math::Vector3& pos, float rad);
		//������񂾂��X�V����ꍇ
		void Update();
		void ViewportActivate();
		void Activate();
		//���[�v�̍Ō�ɌĂ�ł�������
		void FrameEnd();
		DirectX::XMMATRIX GetColumnViewMatrix();
		DirectX::XMMATRIX GetColumnProjectionMatrix();
		DirectX::XMMATRIX GetRowViewMatrix();
		DirectX::XMMATRIX GetRowProjectionMatrix();
		static DirectX::XMMATRIX CreateViewportMatrix(const Math::Vector2& size);
		static DirectX::XMMATRIX GetNowColumnViewMatrix();
		static DirectX::XMMATRIX GetNowColumnProjectionMatrix();
		static DirectX::XMMATRIX GetNowRowViewMatrix();
		static DirectX::XMMATRIX GetNowRowProjectionMatrix();
	};
}
