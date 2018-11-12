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
		Graphics::SpriteRenderer::Render(rwTexture.get(), Math::Vector2(6 * 100.0f, 0.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
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
		rt->Clear(0x00000000);
		rt->Activate();
		view->Activate();
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangeVertexShader(vsX);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(texture, Math::Vector2(), rt->GetTexture()->GetSize(), 0.0f, Math::Vector2(), texture->GetSize(), 0xFFFFFFFF);
		pass2->Clear(0x00000000);
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
	std::shared_ptr<Graphics::RenderTarget>& GaussianFilterPS::GetRenderTarget() { return pass2; }
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
	DepthOfField::DepthOfField(const Math::Vector2& size, float quality) :PostEffect(size, true, DXGI_FORMAT_R8G8B8A8_UNORM) {
		quality = max(1.0f, quality);
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		//縮小バッファを使用
		step0 = std::make_unique<GaussianFilterPS>(size*quality, DXGI_FORMAT_R8G8B8A8_UNORM);
		step0->SetDispersion(0.001f);
		step1 = std::make_unique<GaussianFilterPS>(size*quality, DXGI_FORMAT_R8G8B8A8_UNORM);
		step1->SetDispersion(0.001f);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianDoFPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(12, Graphics::ShaderStageList::PS);
		//初期値
		SetFocus(150.0f);
		useDoF = true;
	}
	void DepthOfField::SetFocus(float range) { info = { range }; }
	void DepthOfField::SetEnable(bool enable) { useDoF = enable; }
	void DepthOfField::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::RenderTarget* color, Graphics::RenderTarget* depth_of_view) {
		if (Input::GetKeyboardKey(DIK_4) == 1)useDoF = !useDoF;
		//ボケ画像二枚作製
		view->Activate();
		rt->Clear(0x00000000);
		rt->Activate();
		if (useDoF) {
			step0->Dispatch(view.get(), rt.get(), color->GetTexture());
			step1->Dispatch(view.get(), rt.get(), step0->GetRenderTarget()->GetTexture());
			cbuffer->Activate(info);
			depth_of_view->GetTexture()->Set(1, Graphics::ShaderStageList::PS);
			step0->Begin(2); step1->Begin(3);
			std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
			Graphics::SpriteRenderer::ChangePixelShader(ps);
			Graphics::SpriteRenderer::Render(color);
			Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
			step0->End(); step1->End();
		}
		else Graphics::SpriteRenderer::Render(color);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		Graphics::Texture::Clean(1, Graphics::ShaderStageList::PS);
		if (active_view)active_view->Activate();
		if (active_buffer)active_buffer->Activate();
	}
	SSMotionBlur::SSMotionBlur(const Math::Vector2& size) :PostEffect(size, true, DXGI_FORMAT_R8G8B8A8_SNORM) {
		sampler = std::make_shared<Graphics::SamplerState>(Graphics::SAMPLER_PRESET::POINT, 16, true);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "ScreenSpaceMotionBlurPS", Graphics::PixelShader::Model::PS_5_0, false);
	}
	void SSMotionBlur::Dispatch(Graphics::Texture* color, Graphics::Texture* depth) {
		rt->Clear(0x00000000);
		rt->Activate();
		depth->Set(1, Graphics::ShaderStageList::PS);
		std::shared_ptr<Graphics::SamplerState> defaultSampler = Graphics::SpriteRenderer::GetSamplerState();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangeSamplerState(sampler);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(color);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::SpriteRenderer::ChangeSamplerState(defaultSampler);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		Graphics::Texture::Clean(1, Graphics::ShaderStageList::PS);
	}
	//---------------------------------------------------------------------------------------------
	HDRPS::ReductionBuffer::ReductionBuffer(const Math::Vector2& scale, DXGI_FORMAT format) :scale(scale) {
		buffer = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 }, format);
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), scale);
	}
	HDRPS::HDRPS(const Math::Vector2& scale, int blur_count) : PostEffect(scale, true, DXGI_FORMAT_R8G8B8A8_UNORM), stepIndex(0), blurCount(blur_count) {
		blumeBuffer = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		blend = std::make_shared<Graphics::BlendState>(Graphics::BLEND_PRESET::ADD, true, false);
		createLuminancePS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "CreateLuminancePS", Graphics::PixelShader::Model::PS_5_0, false);
		toneMapPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "ToneMapPS", Graphics::PixelShader::Model::PS_5_0, false);
		luminanceBuffer = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16_FLOAT);
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), scale);
		int maxScale = max(i_cast(scale.x), i_cast(scale.y));
		Math::Vector2 tempScale = scale;
		for (int i = 0; ; maxScale /= 2) {
			reductionBuffer.push_back(std::make_unique<ReductionBuffer>(tempScale, DXGI_FORMAT_R16_FLOAT));
			tempScale /= 2.0f;
			if (tempScale.x < 1.0f)tempScale.x = 1.0f;
			if (tempScale.y < 1.0f)tempScale.y = 1.0f;
			if (maxScale == 1)break;
		}
		bufferCount = reductionBuffer.size();
		gaussian.resize(blur_count);
		tempScale = scale;
		for (int i = 0; i < blur_count; i++) {
#ifdef GAUSSIAN_PS
			gaussian[i] = std::make_unique<GaussianFilterPS>(tempScale, DXGI_FORMAT_R16G16B16A16_FLOAT);
#else
			gaussian[i] = std::make_unique<GaussianFilterCS>(tempScale, DXGI_FORMAT_R16G16B16A16_FLOAT);
#endif
			tempScale /= 2.0f;
		}
	}
	void HDRPS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::Texture* hdr_texture, Graphics::Texture* color, int step) {
		//ブルーム実行
		DispatchBlume(active_view, active_buffer, hdr_texture, color);
		//平均輝度値のステップを進める
		//現状最初のほうは、平均算出されていない状態なので、その辺りが大丈夫か調査
		for (int i = 0; i < step; i++) {
			CreatMeanLuminanceBuffer();
		}
		//輝度値バッファ作成
		CreateLuminanceBuffer(blumeBuffer->GetTexture());
		//フィルター実行
		viewport->ViewportActivate();
		rt->Clear(0x00000000);
		rt->Activate();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(toneMapPS);
		//輝度情報セット
		luminanceBuffer->GetTexture()->Set(1, Graphics::ShaderStageList::PS);
		//平均輝度値セット
		reductionBuffer[bufferCount - 1]->buffer->GetTexture()->Set(2, Graphics::ShaderStageList::PS);
		Graphics::SpriteRenderer::Render(blumeBuffer->GetTexture());
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		Graphics::Texture::Clean(1, Graphics::ShaderStageList::PS);
		Graphics::Texture::Clean(2, Graphics::ShaderStageList::PS);
		//元に戻す
		active_view->Activate();
		active_buffer->Activate();
	}
	void HDRPS::DispatchBlume(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::Texture* hdr_texture, Graphics::Texture* color) {
		//ブルーム用に、ガウスでぼかす
		for (int i = 0; i < blurCount; i++) {
			Graphics::Texture* tex = nullptr;
			if (i == 0) tex = hdr_texture;
			else tex = gaussian[i - 1]->GetRenderTarget()->GetTexture();
			gaussian[i]->Dispatch(active_view, active_buffer, tex);
		}
		//ブルーム実行
		blumeBuffer->Clear(0x00000000);
		blumeBuffer->Activate();
		viewport->ViewportActivate();
		//とりあえず加算合成で重ねる、強度を変えたくなったらシェーダーで実装する
		Graphics::SpriteRenderer::Render(color);
		auto& defaultBlend = Graphics::SpriteRenderer::GetBlendState();
		//Graphics::SpriteRenderer::ChangeBlendState(blend);
		//for (int i = 0; i < blurCount; i++) {
		//	Graphics::SpriteRenderer::Render(gaussian[i]->GetRenderTarget()->GetTexture());
		//}
		Graphics::SpriteRenderer::ChangeBlendState(defaultBlend);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	//輝度値をバッファへ格納
	void HDRPS::CreateLuminanceBuffer(Graphics::Texture* hdr_texture) {
		viewport->ViewportActivate();
		luminanceBuffer->Clear(0x00000000);
		luminanceBuffer->Activate();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(createLuminancePS);
		Graphics::SpriteRenderer::Render(hdr_texture);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	//平均輝度値算出を1段階進める
	void HDRPS::CreatMeanLuminanceBuffer() {
		reductionBuffer[stepIndex]->viewport->ViewportActivate();
		reductionBuffer[stepIndex]->buffer->Clear(0x0000000);
		reductionBuffer[stepIndex]->buffer->Activate();
		int previousIndex = stepIndex - 1;
		if (previousIndex >= 0)Graphics::SpriteRenderer::Render(reductionBuffer[previousIndex]->buffer.get());
		else Graphics::SpriteRenderer::Render(luminanceBuffer.get());
		stepIndex++;
		if (stepIndex >= bufferCount)stepIndex = 0;
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void HDRPS::DebugRender() {
		for (int i = 0; i < blurCount; i++) {
			Graphics::SpriteRenderer::Render(gaussian[i]->GetRenderTarget()->GetTexture(), Math::Vector2(100.0f, 0.0f)*(7 + i), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), gaussian[i]->GetRenderTarget()->GetTexture()->GetSize(), 0xFFFFFFFF);
		}
		Graphics::SpriteRenderer::Render(luminanceBuffer->GetTexture(), Math::Vector2(100.0f, 0.0f)*(7 + blurCount), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), luminanceBuffer->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::Render(reductionBuffer[bufferCount - 1]->buffer->GetTexture(), Math::Vector2(100.0f, 0.0f)*(7 + blurCount + 1), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), reductionBuffer[bufferCount - 1]->buffer->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::Render(blumeBuffer->GetTexture(), Math::Vector2(100.0f, 0.0f)*(7 + blurCount + 1) + Math::Vector2(0.0f, 100.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), blumeBuffer->GetTexture()->GetSize(), 0xFFFFFFFF);
	}
}