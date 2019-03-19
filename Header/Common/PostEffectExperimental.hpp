#pragma once
class UnorderedAccessView;

namespace Lobelia::Game::Experimental {
	class PostEffect {
	public:
		PostEffect(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		//RenderTarget�쐬���܂���
		PostEffect() = default;
		virtual void Render(const Math::Vector2& pos, const Math::Vector2& size);
		//�|�X�g�G�t�F�N�g���ʂ��e�N�X�`���Ƃ��ăZ�b�g����
		virtual void Begin(int slot);
		//��Еt��
		virtual void End();
	protected:
		std::unique_ptr<Graphics::RenderTarget> rt;
		int slot;
	};
	class GaussianFilter :public PostEffect {
	public:
		GaussianFilter(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilter() = default;
		//�����͕��U��
		void Update(float dispersion);
		//��O���������ۂɂڂ����Ώ�
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget();
		//XY�u���[���ʂ�`��
		void Render(const Math::Vector2& pos, const Math::Vector2& size)override;
		void Begin(int slot);
	private:
		static constexpr const int DIVISION = 4;
	private:
		ALIGN(16) struct Info {
			float weight[DIVISION];
			float width;
			float height;
		};
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
	class SSAO :public PostEffect {
	public:
		SSAO(const Math::Vector2& size);
		//�e�N�X�`�����̂܂ܓn���΂��������̕����ɂ���Δėp���͏オ��
		//useAO�I�v�V�����������̎���AO�}�b�v�͍쐬����Ȃ�
		void Dispatch(Graphics::RenderTarget* active_rt, Graphics::View* active_view, Graphics::RenderTarget* depth);
		void SetEnable(bool enable);
		//�[�x����臒l
		void SetThresholdDepth(float threshold);
		void Render(const Math::Vector2& pos, const Math::Vector2& size)override;
		//�|�X�g�G�t�F�N�g���ʂ��e�N�X�`���Ƃ��ăZ�b�g����
		void Begin(int slot)override;
	private:
		ALIGN(16) struct Info {
			float offsetPerPixel;
			int useAO;
		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::ComputeShader> cs;
		std::shared_ptr<Graphics::Texture> rwTexture;
		std::unique_ptr<UnorderedAccessView> uav;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//Gaussian DoF
	class DepthOfField :public PostEffect {
	public:
		//1.0�܂�
		DepthOfField(const Math::Vector2& size, float quality = 1.0f);
		~DepthOfField() = default;
		void ChangeQuality(float quality);
		void SetFocusRange(float range);
		void SetEnable(bool enable);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::RenderTarget* color, Graphics::RenderTarget* depth_of_view);
		std::shared_ptr<Graphics::RenderTarget> GetStep0();
		std::shared_ptr<Graphics::RenderTarget> GetStep1();
	private:
		ALIGN(16) struct Info {
			float focusRange;
		};
	private:
		const Math::Vector2 size;
		std::unique_ptr<Graphics::View> view;
		//�ア�{�P
		std::unique_ptr<GaussianFilter> step0;
		//��̉摜������ɂڂ������{�P
		std::unique_ptr<GaussianFilter> step1;
		//DoF����
		std::shared_ptr<Graphics::PixelShader> ps;
		//�萔�o�b�t�@
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		bool useDoF;
	};
	class MultipleGaussianBloom {
	public:
		MultipleGaussianBloom(const Math::Vector2& window_size);
		~MultipleGaussianBloom() = default;
		//�E�G�C�g�v�Z���܂�
		void ComputeDispersion(float dispersion);
		std::shared_ptr<Graphics::RenderTarget>& GetGaussianRenderTarget(int index);
		//�Ώۂ̃J���[�o�b�t�@���ڂ��������ʂ��Z�o���܂�
		void Dispatch(std::shared_ptr<Graphics::RenderTarget>& color, Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		void Render(const Math::Vector2& pos, const Math::Vector2& size, bool blend_add = true);
	private:
		std::array<std::unique_ptr<Experimental::GaussianFilter>, 4> gaussian;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::SamplerState> sampler;
		std::shared_ptr<Graphics::BlendState> blend;
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::RenderTarget> rt;
	};
	//�w���g�[���}�b�v���������܂�
	class ToneMap {
	public:
		ToneMap(const Math::Vector2& size);
		~ToneMap() = default;
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget>& color, int step);
		Graphics::Texture* AvgLuminanceTexture();
		void Render(const Math::Vector2& pos, const Math::Vector2& size);
		//�������ݑΏۂ̃����_�[�^�[�Q�b�g
		void DebugRender(Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		//�P�x�������o��
		void ComputeLuminance(std::shared_ptr<Graphics::RenderTarget>& color);
		//�P�x���ώZ�o�X�e�b�v��1�i�K�i�߂܂�
		void ComputeAvgLuminance();
		//�I���x�Z�o���܂�
		void ComputeExposure(Graphics::RenderTarget* active_rt);
		//�g�[���}�b�v�����s���܂�
		void DispatchTonemap(std::shared_ptr<Graphics::RenderTarget>& color,Graphics::View* active_view);
	private:
		struct ReductionBuffer {
		public:
			ReductionBuffer(const Math::Vector2& scale, DXGI_FORMAT format);
			~ReductionBuffer() = default;
		public:
			std::unique_ptr<Graphics::RenderTarget> buffer;
			std::unique_ptr<Graphics::View> viewport;
			Math::Vector2 scale;
		};
		ALIGN(16)struct Info {
			float elapsedTime;
		};
	private:
		//�P�x�i�[�p �P�x���ς��Z�o���邽�߂Ɏg��
		std::unique_ptr<Graphics::RenderTarget> luminance;
		std::unique_ptr<Graphics::View> luminanceView;
		//�P�x�i�[�p
		std::shared_ptr<Graphics::PixelShader> luminanceMapPS;
		//���ϋP�x�Z�o�p
		std::vector<std::unique_ptr<ReductionBuffer>> reductionBuffer;
		int bufferCount;
		int stepIndex;
		std::unique_ptr<RWByteAddressBuffer> byteAddressBuffer;
		std::unique_ptr<Graphics::ComputeShader> computeExposureCS;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		std::shared_ptr<Graphics::PixelShader> tonemapPS;
		//�o�͗p
		std::unique_ptr<Graphics::RenderTarget> rt;
	private://�f�o�b�O�p
		std::shared_ptr<Graphics::PixelShader>  debugExposureRenderPS;
	};
}