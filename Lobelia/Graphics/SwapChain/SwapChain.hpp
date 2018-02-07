#pragma once
namespace Lobelia::Graphics {
	class SwapChain {
		friend class GammaCorrection;
	private:
		ComPtr<IDXGISwapChain> swapChain;
		std::shared_ptr<RenderTarget> renderTarget;
	private:
		void CreateSwapChain(Window* window, DXGI_SAMPLE_DESC sample_desc, int refresh_rate);
	public:
		SwapChain(Window* window, DXGI_SAMPLE_DESC sample_desc, DisplayInfo* info = nullptr);
		~SwapChain();
		const ComPtr<IDXGISwapChain>& Get();
		static DXGI_SAMPLE_DESC SearchLevelMSAA();
		RenderTarget* GetRenderTarget();
		void Clear(Utility::Color color);
		void Present();
		void ResizeBuffer(const Math::Vector2& size);
	};
}