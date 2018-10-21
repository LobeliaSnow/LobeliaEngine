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
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget();
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
	//現状PSのほうが早い。なんでや。
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
		//分散率
		float dispersion;
	};
	class GaussianFilterPS :public PostEffect {
	public:
		GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterPS() = default;
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

}