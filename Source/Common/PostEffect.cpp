#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/DeferredBuffer.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffect.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Post Effect
	//
	//---------------------------------------------------------------------------------------------
	PostEffect::PostEffect(const Math::Vector2& size, bool create_rt, DXGI_FORMAT format) :size(size) {
		if (create_rt)rt = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
	}
	std::shared_ptr<Graphics::RenderTarget>& PostEffect::GetRenderTarget() {
		if (!rt)STRICT_THROW("作成されていないバッファです");
		return rt;
	}
	void PostEffect::Render() { Graphics::SpriteRenderer::Render(rt.get()); }
	void PostEffect::Begin(int slot) { this->slot = slot; }
	//基本的にポストエフェクトはピクセルシェーダーでしか使われない
	void PostEffect::End() { Graphics::Texture::Clean(slot, Graphics::ShaderStageList::PS); }
	//---------------------------------------------------------------------------------------------
	//DXGI_FORMAT_R16G16B16A16_FLOATでするとちらついてバグる。理由不明。
	SSAOCS::SSAOCS(const Math::Vector2& size) :PostEffect(size, true, /*DXGI_FORMAT_R16G16B16A16_FLOAT*/DXGI_FORMAT_R32G32B32A32_FLOAT) {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "SSAOCS");
		//解像度合わせるよう
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		rwTexture = std::make_shared<Graphics::Texture>(size, DXGI_FORMAT_R16_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		uav = std::make_unique<UnorderedAccessView>(rwTexture.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(7, Graphics::ShaderStageList::CS | Graphics::ShaderStageList::PS);
		info.offsetPerPixel = 5.0f;
		info.useAO = TRUE;
		//PS用 使わないパラメーター
		info.offsetPerPixelX = 1.0f; info.offsetPerPixelY = 1.0f;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use AO", &info.useAO, false);
		HostConsole::GetInstance()->FloatRegister("deferred", "use offset per pixel", &info.offsetPerPixel, false);
#endif
	}
	void SSAOCS::CreateAO(Graphics::RenderTarget* active_rt, Graphics::View* active_view, DeferredBuffer* deferred_buffer) {
		//int slot, ID3D11ShaderResourceView* uav
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_8) == 1) info.useAO = !info.useAO;
#endif
		cbuffer->Activate(info);
		if (!info.useAO)return;
		uav->Set(0);
		Graphics::RenderTarget* viewRT = deferred_buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS).get();
		//バッファのコピー
		if (size != viewRT->GetTexture()->GetSize()) {
			rt->Clear(0x00000000);
			rt->Activate();
			view->ViewportActivate();
			Graphics::SpriteRenderer::Render(viewRT);
			active_rt->Activate();
			active_view->Activate();
			rt->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		}
		else viewRT->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		cs->Dispatch(i_cast(size.x + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, i_cast(size.y + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, 1);
		uav->Clean(0);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::CS);
	}
	void SSAOCS::Render() {
		Graphics::SpriteRenderer::Render(rwTexture.get(), Math::Vector2(5 * 200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void SSAOCS::Begin(int slot) {
		PostEffect::Begin(slot);
		rwTexture->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	SSAOPS::SSAOPS(const Math::Vector2& size) :PostEffect(size, true, DXGI_FORMAT_R16_FLOAT) {
		//SSAO書き込み対象
		ps = std::make_unique<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "SSAOPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(7, Graphics::ShaderStageList::PS);
		info.offsetPerPixel = 20.0f;
		info.useAO = TRUE;
		info.offsetPerPixelX = 1.0f / size.x;
		info.offsetPerPixelY = 1.0f / size.y;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use AO", &info.useAO, false);
		HostConsole::GetInstance()->FloatRegister("deferred", "use offset per pixel", &info.offsetPerPixel, false);
#endif
	}
	void SSAOPS::CreateAO(Graphics::RenderTarget* active_rt, DeferredBuffer* deferred_buffer) {
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_8) == 1) info.useAO = !info.useAO;
#endif
		cbuffer->Activate(info);
		if (!info.useAO)return;
		rt->Activate();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(deferred_buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS)->GetTexture());
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		active_rt->Activate();
	}
	void SSAOPS::Render() {
		Graphics::SpriteRenderer::Render(rt.get(), Math::Vector2(5 * 200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void SSAOPS::Begin(int slot) {
		PostEffect::Begin(slot);
		rt->GetTexture()->Set(this->slot, Graphics::ShaderStageList::PS);
	}

	//---------------------------------------------------------------------------------------------
	GaussianFilterCS::GaussianFilterCS(const Math::Vector2& size, DXGI_FORMAT format) :PostEffect(size, true) {
		csX = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterCSX");
		csY = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterCSY");
		rwTexturePass1 = std::make_shared<Graphics::Texture>(size, format, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		rwTexturePass2 = std::make_shared<Graphics::Texture>(size, format, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		uavPass1 = std::make_unique<UnorderedAccessView>(rwTexturePass1.get());
		uavPass2 = std::make_unique<UnorderedAccessView>(rwTexturePass2.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(9, Graphics::ShaderStageList::CS);
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		dispersion = 0.03f;
	}
	void GaussianFilterCS::SetDispersion(float dispersion) { this->dispersion = dispersion; }
	void GaussianFilterCS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture) {
		if (size != texture->GetSize()) {
			//解像度合わせる、基本ダウンサンプリングされる
			this->rt->Clear(0x00000000);
			this->rt->Activate();
			view->ViewportActivate();
			Graphics::SpriteRenderer::Render(texture);
			active_rt->Activate();
			active_view->ViewportActivate();
			this->rt->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		}
		else texture->Set(0, Graphics::ShaderStageList::CS);
		static constexpr const UINT DIVISION = 7;
		if (dispersion <= 0.0f)dispersion = 0.01f;
		float total = 0.0f;
		// ガウス関数による重みの計算
		for (int i = 0; i < DIVISION; i++) {
			float pos = (float)i * 2.0f;
			info.weight[i] = expf(-pos * pos * dispersion);
			total += info.weight[i];
		}
		// 重みの規格化
		for (int i = 0; i < DIVISION; i++) {
			info.weight[i] = info.weight[i] / total * 0.5f;
		}
		cbuffer->Activate(info);
		uavPass1->Set(0);
		csX->Dispatch(i_cast((size.x + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), i_cast((size.y + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), 1);
		uavPass1->Clean(0);
		uavPass2->Set(0);
		rwTexturePass1->Set(0, Graphics::ShaderStageList::CS);
		csY->Dispatch(i_cast((size.x + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), i_cast((size.y + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), 1);
		uavPass2->Clean(0);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::CS);
	}
	void GaussianFilterCS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt) { Dispatch(active_view, active_rt, rt->GetTexture()); }
	void GaussianFilterCS::Render() {
		Graphics::SpriteRenderer::Render(rwTexturePass2.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterCS::Begin(int slot) {
		PostEffect::Begin(slot);
		rwTexturePass2->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterCS::DebugRender(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(rwTexturePass2.get(), pos, size, 0.0f, Math::Vector2(), rwTexturePass2->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	GaussianFilterPS::GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format) :PostEffect(size, true, format) {
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		vsX = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterVSX", Graphics::VertexShader::Model::VS_5_0, false);
		vsY = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterVSY", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterPS", Graphics::PixelShader::Model::PS_5_0, false);
		pass2 = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(9, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		dispersion = 0.03f;
	}
	//分散率の設定
	void GaussianFilterPS::SetDispersion(float dispersion) { this->dispersion = dispersion; }
	//第三引数が実際にぼかす対象
	void GaussianFilterPS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture) {
		info.width = texture->GetSize().x;
		info.height = texture->GetSize().y;
		static constexpr const UINT DIVISION = 7;
		if (dispersion <= 0.0f)dispersion = 0.01f;
		float total = 0.0f;
		// ガウス関数による重みの計算
		for (int i = 0; i < DIVISION; i++) {
			float pos = (float)i * 2.0f;
			info.weight[i] = expf(-pos * pos * dispersion);
			total += info.weight[i];
		}
		// 重みの規格化
		for (int i = 0; i < DIVISION; i++) {
			info.weight[i] = info.weight[i] / total * 0.5f;
		}
		cbuffer->Activate(info);
		rt->Activate();
		view->Activate();
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangeVertexShader(vsX);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(texture, Math::Vector2(), rt->GetTexture()->GetSize(), 0.0f, Math::Vector2(), texture->GetSize(), 0xFFFFFFFF);
		pass2->Activate();
		Graphics::SpriteRenderer::ChangeVertexShader(vsY);
		Graphics::SpriteRenderer::Render(rt.get(), Math::Vector2(), rt->GetTexture()->GetSize(), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		active_view->Activate();
		active_rt->Activate();
	}
	void GaussianFilterPS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt) {
		Dispatch(active_view, active_rt, rt->GetTexture());
	}
	//XYブラー結果を描画
	void GaussianFilterPS::Render() {
		Graphics::SpriteRenderer::Render(pass2.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterPS::Begin(int slot) {
		PostEffect::Begin(slot);
		pass2->GetTexture()->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterPS::DebugRender(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(pass2.get(), pos, size, 0.0f, Math::Vector2(), pass2->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}

}