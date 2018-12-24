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
		SSAOCS(const Math::Vector2& size);
		//�e�N�X�`�����̂܂ܓn���΂��������̕����ɂ���Δėp���͏オ��
		//useAO�I�v�V�����������̎���AO�}�b�v�͍쐬����Ȃ�
		void CreateAO(Graphics::RenderTarget* active_rt, Graphics::View* active_view, class DeferredBuffer* deferred_buffer);
		//AO�𒼐ڕ`�悷�邱�Ƃ͂Ȃ����߁A�f�o�b�O�`������Ă���
		void Render()override;
		void Begin(int slot);
		void SetEnable(bool enable);
		//�[�x����臒l
		void SetThresholdDepth(float threshold);
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
	////����PS�̂ق��������B�Ȃ�ł�B
	//1�p�X�ŏo����\������A����ɂ�鍂����������
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
			float weight[8];
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
	//---------------------------------------------------------------------------------------------
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
	//---------------------------------------------------------------------------------------------
	//Gaussian DoF
	class DepthOfField :public PostEffect {
	public:
		//1.0�܂�
		DepthOfField(const Math::Vector2& size, float quality = 1.0f);
		~DepthOfField() = default;
		void SetFocusRange(float range);
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
	//---------------------------------------------------------------------------------------------
	//GPUGems3
	//����r���[����������Z�b�g����Ă���O��
	class SSMotionBlur :public PostEffect {
	public:
		SSMotionBlur(const Math::Vector2& size);
		~SSMotionBlur() = default;
		void Dispatch(Graphics::Texture* color, Graphics::Texture* depth);
	private:
		std::shared_ptr<Graphics::SamplerState> sampler;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::RenderTarget> temporalRTColor;
	};
	//---------------------------------------------------------------------------------------------
	//HDR�e�N�X�`���ɑ΂��āA�u���[���A�g�[���}�b�v�ƁA�I�������A�K���}�␳���s��(��䊂͍l����)
	//���݁A�����ł���HDR�e�N�X�`���͂��łɋP�x���o����Ă�����̂Ƃ���
	//�K�v�ł���΋P�x���o����������
	class HDRPS :public PostEffect {
	public:
		HDRPS(const Math::Vector2& scale, int blur_count = 4);
		~HDRPS() = default;
		void EnableVignette(bool use_vignette);
		void SetChromaticAberrationIntensity(float chromatic_aberration_intensity);
		void SetRadius2(float radius2);
		void SetSmooth(float smooth);
		void SetMechanicalScale(float mechanical_scale);
		void SetCosFactor(float cos_factor);
		void SetCosPower(float cos_power);
		void SetNaturalScale(float natural_scale);
		//���ϋP�x�l�̌v�Z�����X�e�b�v�i�߂邩
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::Texture* hdr_texture, Graphics::Texture* color, int step = 1);
		void DebugRender();
	private:
		void DispatchBlume(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::Texture* hdr_texture, Graphics::Texture*color);
		//�P�x�l���o�b�t�@�֊i�[
		void CreateLuminanceBuffer(Graphics::Texture* hdr_texture);
		void CreatMeanLuminanceBuffer();
	private:
		class ReductionBuffer {
		public:
			ReductionBuffer(const Math::Vector2& scale, DXGI_FORMAT format);
			~ReductionBuffer() = default;
		public:
			std::unique_ptr<Graphics::RenderTarget> buffer;
			std::unique_ptr<Graphics::View> viewport;
			Math::Vector2 scale;
		};
		ALIGN(16) struct Info {
			//�I���x
			float exposure = 0.5f;
			//�{���������̕␳�l
			float chromaticAberrationIntensity = 0.005f;
			int useVignette = TRUE;
			float radius2 = 4.8f;
			//�Â��Ƃ��납�疾�邭�Ȃ�ۂ̊��炩���A������Ί��炩�ł͂Ȃ��Ȃ�
			//���R�͏�̂ق��ɕ������Ȑ��O���t�ɂȂ�̂�
			float smooth = 1.0f;
			//������ �S�̓I�Ɍ�������
			float mechanicalScale = 1.0f;
			//�R�T�C���l�摥
			float cosFactor = 1.0f;
			float cosPower = 1.0f;
			float naturalScale = 1.0f;
		};
	private:
		//�u���[���p
		std::unique_ptr<Graphics::RenderTarget> colorBuffer;
		std::unique_ptr<Graphics::RenderTarget> blumeBuffer;
		std::shared_ptr<Graphics::PixelShader> blumePS;
		std::shared_ptr<Graphics::BlendState> blend;
		std::shared_ptr<Graphics::SamplerState> sampler;

#ifdef GAUSSIAN_PS
		std::vector<std::unique_ptr<GaussianFilterPS>> gaussian;
#else
		std::vector<std::unique_ptr<GaussianFilterCS>> gaussian;
#endif
		int blurCount;
		//�P�x�l�p
		std::shared_ptr<Graphics::PixelShader> createLuminancePS;
		std::unique_ptr<Graphics::RenderTarget> luminanceBuffer;
		std::unique_ptr<Graphics::View> luminanceViewport;
		std::unique_ptr<Graphics::View> viewport;
		//���ϋP�x�Z�o�p
		std::vector<std::unique_ptr<ReductionBuffer>> reductionBuffer;
		int bufferCount;
		int stepIndex;
		//�t�B���^���s�p
		std::shared_ptr<Graphics::PixelShader> toneMapPS;
		//�R�s�[
		std::shared_ptr<Graphics::Texture> copyTex;
		//�萔�o�b�t�@
		Info info;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};
	//class HDR :public PostEffect {
	//public:
	//	HDR(const Math::Vector2& size);
	//	~HDR() = default;
	//	void Dispatch();
	//private:
	//};
}