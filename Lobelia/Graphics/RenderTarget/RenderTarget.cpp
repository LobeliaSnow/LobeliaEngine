#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config/Config.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Texture/Texture.hpp"


namespace Lobelia::Graphics {
	RenderTarget::RenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, const DXGI_FORMAT& format, int array_count) {
		texture = std::make_shared<Texture>(size, format, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, sample, Texture::ACCESS_FLAG::DEFAULT, Texture::CPU_ACCESS_FLAG::NONE, array_count);
		CreateRenderTarget(size, sample, array_count);
		CreateDepthView();
	}
	RenderTarget::RenderTarget(const ComPtr<ID3D11Texture2D>& texture) {
		this->texture = std::make_shared<Texture>(texture);
		D3D11_TEXTURE2D_DESC desc;
		this->texture->texture->GetDesc(&desc);
		CreateRenderTarget(this->texture->GetSize(), desc.SampleDesc, 1);
		CreateDepthView();
	}
	RenderTarget::~RenderTarget() = default;
	void RenderTarget::CreateRenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, int array_count) {
		HRESULT hr = S_OK;
		if (array_count > 1) {
			D3D11_TEXTURE2D_DESC texDesc;
			texture->texture->GetDesc(&texDesc);
			D3D11_RENDER_TARGET_VIEW_DESC desc;
			desc.Format = texDesc.Format;
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = array_count;
			desc.Texture2DArray.MipSlice = 0;
			hr = Device::Get()->CreateRenderTargetView(texture->texture.Get(), &desc, renderTarget.GetAddressOf());
			if (FAILED(hr))STRICT_THROW("レンダーターゲットビューの作成に失敗しました");
			oneFaceTarget.resize(array_count);
			desc.Texture2DArray.ArraySize = 1;
			for (int i = 0; i < array_count; i++) {
				desc.Texture2DArray.FirstArraySlice = i;
				hr = Device::Get()->CreateRenderTargetView(texture->texture.Get(), &desc, oneFaceTarget[i].GetAddressOf());
				if (FAILED(hr))STRICT_THROW("レンダーターゲットビューの作成に失敗しました");
			}
		}
		else {
			hr = Device::Get()->CreateRenderTargetView(texture->texture.Get(), nullptr, renderTarget.GetAddressOf());
			if (FAILED(hr))STRICT_THROW("レンダーターゲットビューの作成に失敗しました");
		}
		depth = std::make_shared<Texture>(size, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, sample);
	}
	void RenderTarget::CreateDepthView() {
		D3D11_TEXTURE2D_DESC tdesc = {};
		depth->texture->GetDesc(&tdesc);
		D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = tdesc.Format;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		desc.Texture2D.MipSlice = 0;

		HRESULT hr = Device::Get()->CreateDepthStencilView(depth->texture.Get(), &desc, depthView.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("デプスステンシルビューの作成に失敗");
	}
	Texture* RenderTarget::GetTexture()const { return texture.get(); }
	void RenderTarget::Clear(Utility::Color color) {
		//順番が正しいかどうか確認すること
		float clearColor[4] = { color.GetNormalizedR() ,color.GetNormalizedG() ,color.GetNormalizedB(),color.GetNormalizedA() };
		//float clearColor[4] = { color.GetNormalizedB(),color.GetNormalizedG() ,color.GetNormalizedR() ,color.GetNormalizedA() };
		Device::GetContext()->ClearRenderTargetView(renderTarget.Get(), clearColor);
		Device::GetContext()->ClearDepthStencilView(depthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	void RenderTarget::ChangeDepthStencil(RenderTarget* view) {
		depthView = view->depthView;
	}
	void RenderTarget::Activate() {
		Device::GetContext()->OMSetRenderTargets(1, renderTarget.GetAddressOf(), depthView.Get()); 
	}
	void RenderTarget::Activate(int rt_count, RenderTarget** rts) {
		ID3D11RenderTargetView** renderTransform = new ID3D11RenderTargetView*[rt_count];
		for (int i = 0; i < rt_count; i++) {
			renderTransform[i] = rts[i]->renderTarget.Get();
		}
		Device::GetContext()->OMSetRenderTargets(rt_count, renderTransform, rts[0]->depthView.Get());
		Utility::SafeDelete(renderTransform);
	}

}