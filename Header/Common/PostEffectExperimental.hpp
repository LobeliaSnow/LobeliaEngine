#pragma once
class UnorderedAccessView;

namespace Lobelia::Game::Experimental {
	class PostEffect {
	public:
		PostEffect(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		//RenderTarget作成しません
		PostEffect() = default;
		virtual void Render(const Math::Vector2& pos, const Math::Vector2& size);
		//ポストエフェクト結果をテクスチャとしてセットする
		virtual void Begin(int slot);
		//後片付け
		virtual void End();
	protected:
		std::unique_ptr<Graphics::RenderTarget> rt;
		int slot;
	};
	class GaussianFilter :public PostEffect {
	public:
		GaussianFilter(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilter() = default;
		//引数は分散率
		void Update(float dispersion);
		//第三引数が実際にぼかす対象
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget();
		//XYブラー結果を描画
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
		//分散率
		float dispersion;
	};
	class SSAO :public PostEffect {
	public:
		SSAO(const Math::Vector2& size);
		//テクスチャそのまま渡せばいいだけの方式にすれば汎用性は上がる
		//useAOオプションが無効の時はAOマップは作成されない
		void Dispatch(Graphics::RenderTarget* active_rt, Graphics::View* active_view, Graphics::RenderTarget* depth);
		void SetEnable(bool enable);
		//深度差の閾値
		void SetThresholdDepth(float threshold);
		void Render(const Math::Vector2& pos, const Math::Vector2& size)override;
		//ポストエフェクト結果をテクスチャとしてセットする
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
		//1.0まで
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
		//弱いボケ
		std::unique_ptr<GaussianFilter> step0;
		//上の画像をさらにぼかしたボケ
		std::unique_ptr<GaussianFilter> step1;
		//DoF実装
		std::shared_ptr<Graphics::PixelShader> ps;
		//定数バッファ
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		bool useDoF;
	};
	class MultipleGaussianBloom {
	public:
		MultipleGaussianBloom(const Math::Vector2& window_size);
		~MultipleGaussianBloom() = default;
		//ウエイト計算します
		void ComputeDispersion(float dispersion);
		std::shared_ptr<Graphics::RenderTarget>& GetGaussianRenderTarget(int index);
		//対象のカラーバッファをぼかした結果を算出します
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
	//指数トーンマップを実装します
	class ToneMap {
	public:
		ToneMap(const Math::Vector2& size);
		~ToneMap() = default;
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget>& color, int step);
		Graphics::Texture* AvgLuminanceTexture();
		void Render(const Math::Vector2& pos, const Math::Vector2& size);
		//書き込み対象のレンダーターゲット
		void DebugRender(Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		//輝度を書き出す
		void ComputeLuminance(std::shared_ptr<Graphics::RenderTarget>& color);
		//輝度平均算出ステップを1段階進めます
		void ComputeAvgLuminance();
		//露光度算出します
		void ComputeExposure(Graphics::RenderTarget* active_rt);
		//トーンマップを実行します
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
		//輝度格納用 輝度平均を算出するために使う
		std::unique_ptr<Graphics::RenderTarget> luminance;
		std::unique_ptr<Graphics::View> luminanceView;
		//輝度格納用
		std::shared_ptr<Graphics::PixelShader> luminanceMapPS;
		//平均輝度算出用
		std::vector<std::unique_ptr<ReductionBuffer>> reductionBuffer;
		int bufferCount;
		int stepIndex;
		std::unique_ptr<RWByteAddressBuffer> byteAddressBuffer;
		std::unique_ptr<Graphics::ComputeShader> computeExposureCS;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		std::shared_ptr<Graphics::PixelShader> tonemapPS;
		//出力用
		std::unique_ptr<Graphics::RenderTarget> rt;
	private://デバッグ用
		std::shared_ptr<Graphics::PixelShader>  debugExposureRenderPS;
	};
}