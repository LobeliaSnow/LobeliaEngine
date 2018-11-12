#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/DeferredBuffer.hpp"
#include "Common/DeferredShader.hpp"
#include "Common/PostEffect.hpp"

namespace Lobelia::Game {
	DeferredShader::DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps) {
		if (std::string(entry_vs) != "")vs = std::make_shared<Graphics::VertexShader>(file_path, entry_vs, Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>(file_path, entry_ps, Graphics::PixelShader::Model::PS_5_0);
		//blend = std::make_shared<Graphics::BlendState>(Graphics::BLEND_PRESET::ADD, true, false);
		//とりあえずウインドウサイズ準拠
		Math::Vector2 scale = Application::GetInstance()->GetWindow()->GetSize();
		hdr = std::make_unique<HDRPS>(scale);
	}
	void DeferredShader::Render(DeferredBuffer* buffer) {
		buffer->Begin();
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		if (vs)Graphics::SpriteRenderer::ChangeVertexShader(vs);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(buffer->GetRenderTarget(s_cast<DeferredBuffer::BUFFER_TYPE>(0))->GetTexture());
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		buffer->End();
	}
	void DeferredShader::RenderHDR(Graphics::View* active_view, Graphics::RenderTarget* active_rt, DeferredBuffer* buffer) {
		if (!hdrTarget) {
			//とりあえずウインドウサイズ準拠
			Math::Vector2 scale = Application::GetInstance()->GetWindow()->GetSize();
			hdrTarget = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}
		hdrTarget->Clear(0x00000000);
		hdrTarget->Activate();
		Render(buffer);
		active_view->Activate();
		active_rt->Activate();
		hdr->Dispatch(active_view, active_rt, buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::EMISSION_COLOR)->GetTexture(), hdrTarget->GetTexture());
		Graphics::SpriteRenderer::Render(hdr->GetRenderTarget()->GetTexture());
		//auto& defaultBlend = Graphics::SpriteRenderer::GetBlendState();
		//Graphics::SpriteRenderer::ChangeBlendState(blend);
		//Graphics::SpriteRenderer::Render(buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::EMISSION_COLOR)->GetTexture());
		//Graphics::SpriteRenderer::ChangeBlendState(defaultBlend);
	}
	void DeferredShader::DebugRender() {
		hdr->DebugRender();
	}
	SimpleDeferred::SimpleDeferred() :DeferredShader("Data/ShaderFile/3D/deferred.hlsl", "", "SimpleDeferredPS") {
	}
	FullEffectDeferred::FullEffectDeferred() : DeferredShader("Data/ShaderFile/3D/deferred.hlsl", "", "FullDeferredPS") {
		cbuffer = std::make_unique<Graphics::ConstantBuffer<PointLights>>(6, Graphics::ShaderStageList::PS);
	}
	void FullEffectDeferred::SetLightBuffer(int index, const PointLight& p_light) {
		lights.pos[index] = p_light.pos;
		lights.color[index] = p_light.color;
		lights.attenuation[index].x = p_light.attenuation;
	}
	void FullEffectDeferred::SetUseCount(int use_count) { lights.usedLightCount = use_count; }
	void FullEffectDeferred::Update() {
		cbuffer->Activate(lights);
	}
}