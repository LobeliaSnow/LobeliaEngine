#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//���󉓋������������r���̂ŁA�J�X�P�[�h�}�������B
	//Parallel Split Shadow Map (�J�X�P�[�h�n)�����\��
	//�J�X�P�[�h�̎Q�l
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
#ifdef GAUSSIAN_CS
	class GaussianFilterCS;
#endif
#ifdef GAUSSIAN_PS
	class GaussianFilterPS;
#endif
	//��ɍ�蒼��
	//Exponantial Shadow Map��variance Shadow Map�ǂ����������̂���������
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance);
		void AddModel(std::shared_ptr<Graphics::Model> model);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
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
		//void AdjustClipPlanes();
		void ComputeSplit(float lamda, float near_z, float far_z);
		void CameraUpdate();
	private:
		//��ɑ���
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 view;
#ifdef CASCADE
			//4�O��
			std::array<DirectX::XMFLOAT4X4, 4> proj;
			Math::Vector4 pos;
			Math::Vector4 front;
			float splitPos[4];
#else
			std::array<DirectX::XMFLOAT4X4, 1> proj;
#endif
			int useShadowMap;
			int useVariance;
		};
	private:
		std::vector<std::unique_ptr<Graphics::View>> views;
		std::vector<std::shared_ptr<Graphics::RenderTarget>> rts;
		//near/far�������Ă܂�
		std::vector<float> cascadeValues;
		//�����ʒu�������Ă܂�
		std::vector<float> splitPositions;
		std::list<std::weak_ptr<Graphics::Model>> models;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		//std::unique_ptr<Graphics::SamplerState> sampler;
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
		const int count;
	};

}