#include "Lobelia.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffectExperimental.hpp"
#include "../Data/ShaderFile/Define.h"

namespace Lobelia::Game::Experimental {
	PostEffect::PostEffect(const Math::Vector2& size, DXGI_FORMAT format) {
		rt = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
	}
	void PostEffect::Render(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(rt.get(), pos, size, 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void PostEffect::Begin(int slot) {
		this->slot = slot;
		rt->GetTexture()->Set(slot, Graphics::ShaderStageList::PS);
	}
	void PostEffect::End() {
		Graphics::Texture::Clean(slot, Graphics::ShaderStageList::PS);
	}
	GaussianFilter::GaussianFilter(const Math::Vector2& size, DXGI_FORMAT format) :PostEffect(size, format) {
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		vsX = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "GaussianFilterVSX", Graphics::VertexShader::Model::VS_5_0, false);
		vsY = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "GaussianFilterVSY", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "GaussianFilterPS", Graphics::PixelShader::Model::PS_5_0, false);
		pass2 = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(9, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		dispersion = 0.03f;
	}
	//分散率の設定
	void GaussianFilter::Update(float dispersion) {
		if (dispersion <= 0.0f) dispersion = 0.01f;
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
	}
	//第三引数が実際にぼかす対象
	void GaussianFilter::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture) {
		info.width = texture->GetSize().x;
		info.height = texture->GetSize().y;
		cbuffer->Activate(info);
		rt->Clear(0x00000000);
		//1パス目
		rt->Activate();
		view->Activate();
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangeVertexShader(vsX);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(texture, Math::Vector2(), rt->GetTexture()->GetSize(), 0.0f, Math::Vector2(), texture->GetSize(), 0xFFFFFFFF);
		//2パス目
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
	void GaussianFilter::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt) {
		Dispatch(active_view, active_rt, rt->GetTexture());
	}
	std::shared_ptr<Graphics::RenderTarget>& GaussianFilter::GetRenderTarget() { return pass2; }
	//XYブラー結果を描画
	void GaussianFilter::Render(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(pass2.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void GaussianFilter::Begin(int slot) {
		PostEffect::Begin(slot);
		pass2->GetTexture()->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	SSAO::SSAO(const Math::Vector2& size) :PostEffect(size, DXGI_FORMAT_R32_FLOAT) {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "SSAO");
		//解像度合わせるよう
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		rwTexture = std::make_shared<Graphics::Texture>(size, DXGI_FORMAT_R16_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		uav = std::make_unique<UnorderedAccessView>(rwTexture.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(7, Graphics::ShaderStageList::CS | Graphics::ShaderStageList::PS);
		info.offsetPerPixel = 5.0f;
		info.useAO = TRUE;
	}
	void SSAO::Dispatch(Graphics::RenderTarget* active_rt, Graphics::View* active_view, Graphics::RenderTarget* depth) {
		cbuffer->Activate(info);
		if (!info.useAO)return;
		uav->Set(0);
		Graphics::RenderTarget* viewRT = depth;
		//バッファのコピー
		Math::Vector2 size = rt->GetTexture()->GetSize();
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
	void SSAO::SetEnable(bool enable) { info.useAO = int(enable); }
	void SSAO::SetThresholdDepth(float threshold) { info.offsetPerPixel = threshold; }
	void SSAO::Render(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(rwTexture.get(), pos, size, 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void SSAO::Begin(int slot) {
		this->slot = slot;
		rwTexture->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	DepthOfField::DepthOfField(const Math::Vector2& size, float quality) :PostEffect(size, DXGI_FORMAT_R8G8B8A8_UNORM), size(size) {
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		ChangeQuality(quality);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "GaussianDoFPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(12, Graphics::ShaderStageList::PS);
		//初期値
		SetFocusRange(150.0f);
		useDoF = true;
	}
	void DepthOfField::ChangeQuality(float quality) {
		quality = min(1.0f, max(quality, 0.1f));
		//縮小バッファを使用
		step0 = std::make_unique<GaussianFilter>(size*quality, DXGI_FORMAT_R8G8B8A8_UNORM);
		step0->Update(0.1f);
		//強ぼかし クオリティが低い場合、さらに縮小
		if (quality < 1.0f)quality *= 0.5f;
		step1 = std::make_unique<GaussianFilter>(size*quality, DXGI_FORMAT_R8G8B8A8_UNORM);
		if (quality < 1.0f)step1->Update(0.5f);
		else step1->Update(0.1f);
	}
	void DepthOfField::SetFocusRange(float range) { info = { range }; }
	void DepthOfField::SetEnable(bool enable) { useDoF = enable; }
	void DepthOfField::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_buffer, Graphics::RenderTarget* color, Graphics::RenderTarget* depth_of_view) {
		//if (Input::GetKeyboardKey(DIK_4) == 1)useDoF = !useDoF;
		//ボケ画像二枚作製
		view->Activate();
		rt->Clear(0x00000000);
		rt->Activate();
		if (useDoF) {
			//被写界深度用の弱、強ぼかしの作成
			step0->Dispatch(view.get(), rt.get(), color->GetTexture());
			step1->Dispatch(view.get(), rt.get(), step0->GetRenderTarget()->GetTexture());
			//被写界深度実行
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
	std::shared_ptr<Graphics::RenderTarget> DepthOfField::GetStep0() { return step0->GetRenderTarget(); }
	std::shared_ptr<Graphics::RenderTarget> DepthOfField::GetStep1() { return step1->GetRenderTarget(); }
	MultipleGaussianBloom::MultipleGaussianBloom(const Math::Vector2& window_size) {
		Math::Vector2 size = window_size;
		for (int i = 0; i < gaussian.size(); i++) {
			size *= 0.5f;
			gaussian[i] = std::make_unique<Experimental::GaussianFilter>(size, DXGI_FORMAT_R16G16B16A16_FLOAT);
			gaussian[i]->Update(0.001f);
		}
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "AvgColor4PS", Graphics::PixelShader::Model::PS_5_0);
		sampler = std::make_shared<Graphics::SamplerState>(Graphics::SAMPLER_PRESET::LINEAR, 16, true);
		blend = std::make_shared<Graphics::BlendState>(Graphics::BLEND_PRESET::ADD, true, false);
		view = std::make_unique<Graphics::View>(Math::Vector2(), window_size);
		//RenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		rt = std::make_unique<Graphics::RenderTarget>(window_size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}
	void MultipleGaussianBloom::ComputeDispersion(float dispersion) {
		for (int i = 0; i < gaussian.size(); i++) {
			gaussian[i]->Update(dispersion);
		}
	}
	std::shared_ptr<Graphics::RenderTarget>& MultipleGaussianBloom::GetGaussianRenderTarget(int index) {
		if (s_cast<UINT>(index) >= 4)STRICT_THROW("範囲外のインデックスが指定されました");
		return gaussian[index]->GetRenderTarget();
	}
	void MultipleGaussianBloom::Dispatch(std::shared_ptr<Graphics::RenderTarget>& color, Graphics::View* active_view, Graphics::RenderTarget* active_rt) {
		std::shared_ptr<Graphics::SamplerState> defaultSampler = Graphics::SpriteRenderer::GetSamplerState();
		Graphics::SpriteRenderer::ChangeSamplerState(sampler);
		for (int i = 0; i < gaussian.size(); i++) {
			if (i == 0)gaussian[i]->Dispatch(active_view, active_rt, color);
			else gaussian[i]->Dispatch(active_view, active_rt, gaussian[i - 1]->GetRenderTarget());
		}
		Graphics::SpriteRenderer::ChangeSamplerState(defaultSampler);
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		for (int i = 0; i < gaussian.size(); i++) {
			gaussian[i]->Begin(i + 1);
		}
		rt->Clear(0x00000000);
		rt->Activate();
		view->ViewportActivate();
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(color.get(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		for (int i = 0; i < gaussian.size(); i++) {
			gaussian[i]->End();
		}
		active_view->ViewportActivate();
		active_rt->Activate();
	}
	void MultipleGaussianBloom::Render(const Math::Vector2& pos, const Math::Vector2& size, bool blend_add) {
		std::shared_ptr<Graphics::BlendState> defaultBlend = Graphics::SpriteRenderer::GetBlendState();
		if (blend_add)Graphics::SpriteRenderer::ChangeBlendState(blend);
		Graphics::SpriteRenderer::Render(rt.get(), pos, size, 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		if (blend_add) Graphics::SpriteRenderer::ChangeBlendState(defaultBlend);
	}
	ToneMap::ToneMap(const Math::Vector2& size) :stepIndex(0) {
		//最終的に平均輝度算出に使うだけなので、多少小さくても変わらない
		luminance = std::make_unique<Graphics::RenderTarget>(size*0.5f, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16_FLOAT);
		luminanceView = std::make_unique<Graphics::View>(Math::Vector2(), size*0.5f);
		luminanceMapPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "CreateLuminancePS", Graphics::PixelShader::Model::PS_5_0);
		int maxScale = max(i_cast(size.x), i_cast(size.y));
		Math::Vector2 tempScale = size;
		for (int i = 0; ; maxScale /= 2) {
			DXGI_FORMAT format = DXGI_FORMAT_R16_FLOAT;
			reductionBuffer.push_back(std::make_unique<ReductionBuffer>(tempScale, format));
			tempScale /= 2.0f;
			if (tempScale.x < 1.0f)tempScale.x = 1.0f;
			if (tempScale.y < 1.0f)tempScale.y = 1.0f;
			tempScale.x = i_cast(tempScale.x); tempScale.y = i_cast(tempScale.y);
			if (maxScale == 1)break;
		}
		bufferCount = reductionBuffer.size();
		//RWByteAddressBuffer(void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
		float init = 0;
		byteAddressBuffer = std::make_unique<RWByteAddressBuffer>(r_cast<void*>(&init), sizeof(float), 1, false, false, false);
		computeExposureCS = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "ComputeExposure");
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(13, Graphics::ShaderStageList::CS);
		debugExposureRenderPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "RenderExposurePS", Graphics::PixelShader::Model::PS_5_0);
		tonemapPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffectExperimental.hlsl", "ExpTonemapPS", Graphics::PixelShader::Model::PS_5_0);
		rt = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	ToneMap::ReductionBuffer::ReductionBuffer(const Math::Vector2& scale, DXGI_FORMAT format) {
		buffer = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 }, format);
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), scale);
	}
	void ToneMap::ComputeLuminance(std::shared_ptr<Graphics::RenderTarget>& color) {
		luminance->Clear(0x00000000);
		luminance->Activate();
		luminanceView->ViewportActivate();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(luminanceMapPS);
		Graphics::SpriteRenderer::Render(color.get(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void ToneMap::ComputeAvgLuminance() {
		reductionBuffer[stepIndex]->viewport->ViewportActivate();
		reductionBuffer[stepIndex]->buffer->Clear(0x0000000);
		reductionBuffer[stepIndex]->buffer->Activate();
		int previousIndex = stepIndex - 1;
		if (previousIndex >= 0)Graphics::SpriteRenderer::Render(reductionBuffer[previousIndex]->buffer.get());
		else Graphics::SpriteRenderer::Render(luminance.get());
		stepIndex++;
		if (stepIndex >= bufferCount)stepIndex = 0;
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void ToneMap::ComputeExposure(Graphics::RenderTarget* active_rt) {
		//この先露光度算出
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(1, 1, byteAddressBuffer->uav.GetAddressOf(), nullptr);
		active_rt->Activate();
		AvgLuminanceTexture()->Set(0, Graphics::ShaderStageList::CS);
		info.elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		cbuffer->Activate(info);
		computeExposureCS->Dispatch(1, 1, 1);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::CS);
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
	}
	void ToneMap::DispatchTonemap(std::shared_ptr<Graphics::RenderTarget>& color, Graphics::View* active_view) {
		rt->Clear(0x00000000);
		rt->ActivateUAV(byteAddressBuffer->uav.GetAddressOf(), 1);
		active_view->ViewportActivate();
		AvgLuminanceTexture()->Set(1, Graphics::ShaderStageList::PS);
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(tonemapPS);
		Graphics::SpriteRenderer::Render(color.get(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::Texture::Clean(1, Graphics::ShaderStageList::PS);
	}
	void ToneMap::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget>& color, int step) {
		ComputeLuminance(color);
		//平均輝度値のステップを進める
		for (int i = 0; i < step; i++) {
			ComputeAvgLuminance();
		}
		//露光度を算出
		//前回手法では、CPUからアクセスできる形式にバッファをコピーして、Mapによるアクセスを試みたが、パイプラインストールが発生
		//今回はすべてGPUで完結させるために、ComputeShaderを用いて露光度を算出した
		ComputeExposure(active_rt);
		//本体
		DispatchTonemap(color, active_view);
		//クリーン
		active_view->ViewportActivate();
		active_rt->Activate();
	}
	Graphics::Texture* ToneMap::AvgLuminanceTexture() { return reductionBuffer[bufferCount - 1]->buffer->GetTexture(); }
	void ToneMap::Render(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(rt.get(), pos, size, 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void ToneMap::DebugRender(Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size) {
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(debugExposureRenderPS);
		rt->ActivateUAV(byteAddressBuffer->uav.GetAddressOf(), 1);
		Graphics::SpriteRenderer::Render(rt, pos, size, 0.0f, Math::Vector2(), Math::Vector2(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		rt->Activate();
	}
}