#include "Common/Common.hpp"
#include "Math/Math.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config/Config.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/DisplayInfo/DisplayInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Graphics/SwapChain/SwapChain.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	SwapChain::SwapChain(Window* window, DXGI_SAMPLE_DESC sample_desc, DisplayInfo* info) {
		if (!window)STRICT_THROW("インスタンス対象が設定されていません");
		int refreshRate = 60;
		if (info)refreshRate = info->GetRefreshRate();
		CreateSwapChain(window, sample_desc, refreshRate);
		ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("バックバッファの取得に失敗しました");
		renderTarget = std::make_shared<RenderTarget>(backBuffer);
		renderTarget->Activate();
	}
	SwapChain::~SwapChain() { swapChain->SetFullscreenState(false, nullptr); }
	const ComPtr<IDXGISwapChain>& SwapChain::Get() { return swapChain; }
	void SwapChain::CreateSwapChain(Window* window, DXGI_SAMPLE_DESC sample_desc, int refresh_rate) {
		ComPtr<IDXGIFactory> factory;
		CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = window->GetSize().Get().x;
		desc.BufferDesc.Height = window->GetSize().Get().y;
		desc.BufferDesc.RefreshRate.Numerator = refresh_rate;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		//ここに関しては要相談
		desc.SampleDesc = sample_desc;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = window->GetHandle();
		desc.Windowed = true;
#ifdef _DEBUG
		desc.BufferCount = 1;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
#else
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
#endif
		desc.BufferCount = 1;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		HRESULT hr = factory->CreateSwapChain(Device::Get().Get(), &desc, swapChain.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("スワップチェインの作成に失敗");
	}
	DXGI_SAMPLE_DESC SwapChain::SearchLevelMSAA() {
		HRESULT hr = S_OK;
		DXGI_SAMPLE_DESC msaa = {};
		UINT quality = 0;
		//機能の最大レベル取得
		for (int i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i <<= 1) {
			hr = Device::Get()->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, i, &quality);
			if (SUCCEEDED(hr) && quality > 0) {
				msaa.Count = i;
				msaa.Quality = quality - 1;
			}
		}
		return msaa;
	}
	RenderTarget* SwapChain::GetRenderTarget() { return renderTarget.get(); }
	void SwapChain::Clear(Utility::Color color) { renderTarget->Clear(color); }
	void SwapChain::Present() { swapChain->Present(Config::GetRefPreference().useVSync, 0); }
	void SwapChain::ResizeBuffer(const Math::Vector2& size) {
		renderTarget.reset();
		DXGI_SWAP_CHAIN_DESC desc = {};
		swapChain->GetDesc(&desc);
		swapChain->ResizeBuffers(desc.BufferCount, 0, 0, desc.BufferDesc.Format, desc.Flags);
		ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		renderTarget = std::make_shared<RenderTarget>(backBuffer);
		renderTarget->Activate();
	}
}