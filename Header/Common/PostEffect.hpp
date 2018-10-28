#pragma once
class UnorderedAccessView;
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Post Effect
	//
	//---------------------------------------------------------------------------------------------
	class PostEffect abstract {
	public:
		PostEffect(const Math::Vector2& size, bool create_rt, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		virtual ~PostEffect() = default;
		virtual std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget();
		//�f�t�H���g�̓���́Art�̕`��
		virtual void Render();
		//�|�X�g�G�t�F�N�g���e�N�X�`���Ƃ��ăZ�b�g����
		virtual void Begin(int slot);
		//��Еt��
		virtual void End();
	protected:
		std::shared_ptr<Graphics::RenderTarget> rt;
		Math::Vector2 size;
		int slot;
	};
	//---------------------------------------------------------------------------------------------
	//�u���[�p�X�͏Ȃ��Ă��܂�
	//Compute Shader�ɂ�����
	//�p�t�H�[�}���X�v������
	//x1280 / y 720��
	//AO���� 9.2ms/��108FPS
	//AO�L�� 10.2ms/��98FPS
	//���1ms
	class SSAOCS :public PostEffect {
		friend class SSAOPS;
	public:
		//TODO : �𑜓x�������悤�ɂ���
		SSAOCS(const Math::Vector2& size);
		//�e�N�X�`�����̂܂ܓn���΂��������̕����ɂ���Δėp���͏オ��
		//useAO�I�v�V�����������̎���AO�}�b�v�͍쐬����Ȃ�
		void CreateAO(Graphics::RenderTarget* active_rt, Graphics::View* active_view, DeferredBuffer* deferred_buffer);
		//AO�𒼐ڕ`�悷�邱�Ƃ͂Ȃ����߁A�f�o�b�O�`������Ă���
		void Render()override;
		void Begin(int slot);
	private:
		ALIGN(16) struct Info {
			float offsetPerPixel;
			int useAO;
			float offsetPerPixelX;
			float offsetPerPixelY;
		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::ComputeShader> cs;
		std::shared_ptr<Graphics::Texture> rwTexture;
		std::unique_ptr<UnorderedAccessView> uav;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//�p�t�H�[�}���X��r�\
	class SSAOPS :public PostEffect {
	public:
		SSAOPS(const Math::Vector2& size);
		void CreateAO(Graphics::RenderTarget* active_rt, DeferredBuffer* deferred_buffer);
		//AO�𒼐ڕ`�悷�邱�Ƃ͂Ȃ����߁A�f�o�b�O�`������Ă���
		void Render()override;
		void Begin(int slot);
	private:
		using Info = SSAOCS::Info;
	private:
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//---------------------------------------------------------------------------------------------
	//������ƕs����邩�� (?)
	//Compute Shader�ɂ�����
	//�p�t�H�[�}���X�v������
	//x640 / y 360��
	//�������Ȃ��� 6.0ms/��165FPS
	//�ڂ����L���� 7.0ms/��138FPS
	//���1.0ms
	//����PS�̂ق��������B�Ȃ�ł�B
	class GaussianFilterCS :public PostEffect {
		friend class GaussianFilterPS;
	public:
		GaussianFilterCS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterCS() = default;
		//���U���̐ݒ�
		void SetDispersion(float dispersion);
		//��O���������ۂɂڂ����Ώ�
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XY�u���[���ʂ�`��
		void Render()override;
		void Begin(int slot);
		void DebugRender(const Math::Vector2& pos, const Math::Vector2& size);
	private:
		ALIGN(16) struct Info {
			float weight[7];
			float width;
			float height;
		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::ComputeShader> csX;
		std::unique_ptr<Graphics::ComputeShader> csY;
		std::shared_ptr<Graphics::Texture> rwTexturePass1;
		std::shared_ptr<Graphics::Texture> rwTexturePass2;
		std::unique_ptr<UnorderedAccessView> uavPass1;
		std::unique_ptr<UnorderedAccessView> uavPass2;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		//���U��
		float dispersion;
	};
	class GaussianFilterPS :public PostEffect {
	public:
		GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterPS() = default;
		//���U���̐ݒ�
		void SetDispersion(float dispersion);
		//��O���������ۂɂڂ����Ώ�
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget()override;
		//XY�u���[���ʂ�`��
		void Render()override;
		void Begin(int slot);
		void DebugRender(const Math::Vector2& pos, const Math::Vector2& size);
	private:
		using Info = GaussianFilterCS::Info;
	private:
		std::unique_ptr<Graphics::View> view;
		std::shared_ptr<Graphics::VertexShader> vsX;
		std::shared_ptr<Graphics::VertexShader> vsY;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::RenderTarget> pass2;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		//���U��
		float dispersion;
	};
	//Gaussian DoF
	class DepthOfField :public PostEffect {
	public:
		//1.0�܂�
		DepthOfField(const Math::Vector2& size, float quality = 1.0f);
		~DepthOfField() = default;
		void SetFocus(float range);
		void SetEnable(bool enable);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::RenderTarget* color, Graphics::RenderTarget* depth_of_view);
	private:
		ALIGN(16) struct Info {
			float focusRange;
		};
	private:
		std::unique_ptr<Graphics::View> view;
		//�ア�{�P
		std::unique_ptr<GaussianFilterPS> step0;
		//��̉摜������ɂڂ������{�P
		std::unique_ptr<GaussianFilterPS> step1;
		//DoF����
		std::shared_ptr<Graphics::PixelShader> ps;
		//�萔�o�b�t�@
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		bool useDoF;
	};
}