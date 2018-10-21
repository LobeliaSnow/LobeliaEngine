#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/DeferredShader.hpp"

namespace Lobelia::Game {
	DeferredShader::DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps) {
		if (std::string(entry_vs) != "")vs = std::make_shared<Graphics::VertexShader>(file_path, entry_vs, Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>(file_path, entry_ps, Graphics::PixelShader::Model::PS_5_0);
	}
	void DeferredShader::Render() {
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		if (vs)Graphics::SpriteRenderer::ChangeVertexShader(vs);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(static_cast<Graphics::Texture*>(nullptr));
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
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