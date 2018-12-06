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
		//デフォルトの動作は、rtの描画
		virtual void Render();
		//ポストエフェクトをテクスチャとしてセットする
		virtual void Begin(int slot);
		//後片付け
		virtual void End();
	protected:
		std::shared_ptr<Graphics::RenderTarget> rt;
		Math::Vector2 size;
		int slot;
	};
	//---------------------------------------------------------------------------------------------
	//ブラーパスは省いています
	//Compute Shaderによる実装
	//パフォーマンス計測結果
	//x1280 / y 720で
	//AO無し 9.2ms/約108FPS
	//AO有り 10.2ms/約98FPS
	//大体1ms
	class SSAOCS :public PostEffect {
		friend class SSAOPS;
	public:
		//TODO : 解像度下げれるようにする
		SSAOCS(const Math::Vector2& size);
		//テクスチャそのまま渡せばいいだけの方式にすれば汎用性は上がる
		//useAOオプションが無効の時はAOマップは作成されない
		void CreateAO(Graphics::RenderTarget* active_rt, Graphics::View* active_view, DeferredBuffer* deferred_buffer);
		//AOを直接描画することはないため、デバッグ描画を入れている
		void Render()override;
		void Begin(int slot);
		void SetEnable(bool enable);
		//深度差の閾値
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
	//パフォーマンス比較表
	class SSAOPS :public PostEffect {
	public:
		SSAOPS(const Math::Vector2& size);
		void CreateAO(Graphics::RenderTarget* active_rt, DeferredBuffer* deferred_buffer);
		//AOを直接描画することはないため、デバッグ描画を入れている
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
	//ちょっと不具合あるかも (?)
	//Compute Shaderによる実装
	//パフォーマンス計測結果
	//x640 / y 360で
	//何もしない時 6.0ms/約165FPS
	//ぼかし有効化 7.0ms/約138FPS
	//大体1.0ms
	////現状PSのほうが早い。なんでや。
	//1パスで出来る可能性あり、それによる高速化を試す
	class GaussianFilterCS :public PostEffect {
		friend class GaussianFilterPS;
	public:
		GaussianFilterCS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterCS() = default;
		//分散率の設定
		void SetDispersion(float dispersion);
		//第三引数が実際にぼかす対象
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XYブラー結果を描画
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
		//分散率
		float dispersion;
	};
	//---------------------------------------------------------------------------------------------
	class GaussianFilterPS :public PostEffect {
	public:
		GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterPS() = default;
		//分散率の設定
		void SetDispersion(float dispersion);
		//第三引数が実際にぼかす対象
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget()override;
		//XYブラー結果を描画
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
		//分散率
		float dispersion;
	};
	//---------------------------------------------------------------------------------------------
	//Gaussian DoF
	class DepthOfField :public PostEffect {
	public:
		//1.0まで
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
		//弱いボケ
		std::unique_ptr<GaussianFilterPS> step0;
		//上の画像をさらにぼかしたボケ
		std::unique_ptr<GaussianFilterPS> step1;
		//DoF実装
		std::shared_ptr<Graphics::PixelShader> ps;
		//定数バッファ
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		bool useDoF;
	};
	//---------------------------------------------------------------------------------------------
	//GPUGems3
	//現状ビューがしっかりセットされている前提
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
	//HDRテクスチャに対して、ブルーム、トーンマップと、露光調整、ガンマ補正を行う(光芒は考え中)
	//現在、ここでいうHDRテクスチャはすでに輝度抽出されているものとする
	//必要であれば輝度抽出も実装する
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
		//平均輝度値の計算を何ステップ進めるか
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::Texture* hdr_texture, Graphics::Texture* color, int step = 1);
		void DebugRender();
	private:
		void DispatchBlume(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::Texture* hdr_texture, Graphics::Texture*color);
		//輝度値をバッファへ格納
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
			//露光度
			float exposure = 0.5f;
			//倍率式収差の補正値
			float chromaticAberrationIntensity = 0.005f;
			int useVignette = TRUE;
			float radius2 = 4.8f;
			//暗いところから明るくなる際の滑らかさ、下がれば滑らかではなくなる
			//理由は上のほうに浮いた曲線グラフになるので
			float smooth = 1.0f;
			//減光量 全体的に減光する
			float mechanicalScale = 1.0f;
			//コサイン四乗則
			float cosFactor = 1.0f;
			float cosPower = 1.0f;
			float naturalScale = 1.0f;
		};
	private:
		//ブルーム用
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
		//輝度値用
		std::shared_ptr<Graphics::PixelShader> createLuminancePS;
		std::unique_ptr<Graphics::RenderTarget> luminanceBuffer;
		std::unique_ptr<Graphics::View> luminanceViewport;
		std::unique_ptr<Graphics::View> viewport;
		//平均輝度算出用
		std::vector<std::unique_ptr<ReductionBuffer>> reductionBuffer;
		int bufferCount;
		int stepIndex;
		//フィルタ実行用
		std::shared_ptr<Graphics::PixelShader> toneMapPS;
		//コピー
		std::shared_ptr<Graphics::Texture> copyTex;
		//定数バッファ
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