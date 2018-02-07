#pragma once
namespace Lobelia::Graphics {
	//フルスクのみ使えます
	class GammaCorrection {
	private:
		ComPtr<IDXGIOutput> output;
		DXGI_GAMMA_CONTROL_CAPABILITIES gammacap;
		DXGI_GAMMA_CONTROL gammaControl;
	public:
		GammaCorrection(SwapChain* swap_chain);
		~GammaCorrection();
		//Color型のアルファは無視されます
		void CurveSetting(float gamma, Utility::Color color_scale, Utility::Color color_offset);
		void Set();
	};
}