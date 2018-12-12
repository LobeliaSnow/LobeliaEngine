#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//���󉓋������������r���̂ŁA�J�X�P�[�h�}�������B
	//Parallel Split Shadow Map (�J�X�P�[�h�n)�����\��
	//�J�X�P�[�h
	//�J�X�P�[�h�̎Q�l
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
	//�`��p�̃J�����̎�����𕪊����A���C�g�̈ʒu����e����������S�ɓ���悤�ɃJ������u���ăV���h�E�}�b�v�����
#ifdef GAUSSIAN_CS
	class GaussianFilterCS;
#endif
#ifdef GAUSSIAN_PS
	class GaussianFilterPS;
#endif
	struct AABB {
		Math::Vector3 min;
		Math::Vector3 max;
	};
	//��ɍ�蒼��
	//�J�X�P�[�h�̕����镔���Ŏ኱�e��������ꍇ������
	//Exponantial Shadow Map�����킹����
	//near,far�����ꂢ�ɂȂ�悤�ɒ�������K�v�����邪�A
	//�\�������I�u�W�F�N�g��AABB���K�v�ƂȂ�̂ō���̓X���[
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance);
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		void SetNearPlane(float near_z);
		void SetFarPlane(float far_z);
		void SetLamda(float lamda);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		void SetVariance(bool use_variance);
		void SetEnable(bool use_shadow);
		void CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//�V���h�E�}�b�v���Z�b�g����
		//�e�N�X�`���X���b�g6~6+split_count�Z�b�g����
		//�R���X�^���g�o�b�t�@��
		//ALIGN(16) struct Info {
		//	DirectX::XMFLOAT4X4 view;
		//	DirectX::XMFLOAT4X4 proj[split_count];
		//#ifdef CASCADE
		//	Math::Vector4 lpos;
		// Math::Vector4 ldir;
		//	Math::Vector4 splitPos;
		//#endif
		//	int useShadowMap;
		//	int useVariance;
		//};
		//�̌`���Ńo�C���h�����
		void Begin();
		void End();
		void DebugRender();
	private:
		//�������AABB�v�Z
		AABB CalcFrustumAABB(Graphics::View* main_camera, float near_z, float far_z, const DirectX::XMFLOAT4X4& lvp);
		//void AdjustClipPlanes();
		void ComputeSplit(float lamda, float near_z, float far_z, float* split_pos);
		DirectX::XMFLOAT4X4 FrustumAABBClippingMatrix(AABB clip_aabb);
		void CreateCascade(Graphics::View* main_camera, const DirectX::XMFLOAT4X4& lvp);
		void CameraUpdate(Graphics::View* main_camera);
	private:
		//��ɑ���
		ALIGN(16) struct Info {
#ifdef CASCADE
			//4�O��
			std::array<DirectX::XMFLOAT4X4, 4> cascadeLVP;
			Math::Vector4 pos;
			Math::Vector4 front;
			float splitRange[4];
#else
			DirectX::XMFLOAT4X4 lvp;
#endif
			int useShadowMap;
			int useVariance;
			int nowIndex;
		};
	private:
		std::vector<std::unique_ptr<Graphics::View>> views;
		std::vector<std::shared_ptr<Graphics::RenderTarget>> rts;
		std::list<std::weak_ptr<Graphics::Model>> models;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		std::unique_ptr<Graphics::SamplerState> sampler;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
#ifdef GAUSSIAN_CS
		std::vector<std::unique_ptr<GaussianFilterCS>> gaussian;
#endif
#ifdef GAUSSIAN_PS
		std::vector<std::unique_ptr<GaussianFilterPS>> gaussian;
#endif
		Info info;
		Math::Vector2 size;
		float fov;
		float aspect;
		float nearZ;
		float farZ;
		float lamda;
		const int count;
	};

}