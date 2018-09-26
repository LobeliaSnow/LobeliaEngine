#pragma once
namespace Lobelia::Graphics {
	class Texture;
	class RenderTarget {
	private:
		std::shared_ptr<Texture> texture;
		//レンダーターゲット配列の場合は六枚分を一度にこれで作る
		//一枚の場合はこれがレンダーターゲットそのもの
		ComPtr<ID3D11RenderTargetView> renderTarget;
		//レンダーターゲット配列を使用する場合のみ使う。
		//↑の情報に一枚ずつアクセスできるようにする
		std::vector<ComPtr<ID3D11RenderTargetView>> oneFaceTarget;
		std::shared_ptr<Texture> depth;
		ComPtr<ID3D11DepthStencilView> depthView;
	private:
		void CreateRenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, int array_count);
		void CreateDepthView(int array_count);
	public:
		RenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample = Config::GetRefPreference().msaa, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		RenderTarget(const ComPtr<ID3D11Texture2D>& texture);
		~RenderTarget();
		Texture* GetTexture()const;
		void Clear(Utility::Color color);
		void ChangeDepthStencil(RenderTarget* view);
		void Activate();
		//TODO : 複数同時にセットできるように(MRT)
		//深度バッファは0番目のだけ使われます
		static void Activate(int rt_count, RenderTarget** rts);
	};

}