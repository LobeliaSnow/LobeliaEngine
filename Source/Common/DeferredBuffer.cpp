#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/DeferredBuffer.hpp"
#include "Common/SkyBox.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Geometry Buffer
	//
	//---------------------------------------------------------------------------------------------
	DeferredBuffer::DeferredBuffer(const Math::Vector2& size) :size(size) {
		//for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
		//	rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		//}
		rts[i_cast(BUFFER_TYPE::POS)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		//����ɑ��x�d���Ȃ�A���x�͂�΂������������DXGI_FORMAT_R11G11B10_FLOAT
		rts[i_cast(BUFFER_TYPE::NORMAL)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		rts[i_cast(BUFFER_TYPE::COLOR)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_UNORM);
		rts[i_cast(BUFFER_TYPE::VIEW_POS)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R32G32B32A32_FLOAT);
		rts[i_cast(BUFFER_TYPE::SHADOW)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16_FLOAT);
		rts[i_cast(BUFFER_TYPE::EMISSION_COLOR)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		rts[i_cast(BUFFER_TYPE::MATERIAL_ID)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16_FLOAT);
		//VertexShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage = false);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateGBufferVS", Graphics::VertexShader::Model::VS_4_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateGBufferPS", Graphics::PixelShader::Model::PS_4_0);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(8, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
	}
	std::shared_ptr<Graphics::RenderTarget>& DeferredBuffer::GetRenderTarget(BUFFER_TYPE type) { return rts[i_cast(type)]; }
	void DeferredBuffer::AddModel(std::shared_ptr<Graphics::Model> model, MATERIAL_TYPE material_type, float specular_factor, float emission_factor) {
		models.push_back(ModelStorage{ model,material_type,specular_factor,emission_factor });
	}
	void DeferredBuffer::SetSkybox(std::shared_ptr<SkyBox>& skybox) { this->skybox = skybox; }
	void DeferredBuffer::RenderGBuffer() {
		End();
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			rts[i]->Clear(0x00000000);
		}
		Graphics::RenderTarget::Activate(rts[0].get(), rts[1].get(), rts[2].get(), rts[3].get(), rts[4].get(), rts[5].get(), rts[6].get());
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		if (skybox) {
			cbuffer->Activate(Info{ MATERIAL_TYPE::COLOR,0.0f,0.0f });
			skybox->Render();
		}
		for (auto&& weak : models) {
			if (weak.model.expired())continue;
			std::shared_ptr<Graphics::Model> model = weak.model.lock();
			weak.info.transparent = 0.6f;
			cbuffer->Activate(weak.info);
			model->ChangeAnimVS(vs);
			model->Render();
		}
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		models.clear();
	}
	void DeferredBuffer::Begin() {
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			rts[i]->GetTexture()->Set(i, Graphics::ShaderStageList::PS);
		}
	}
	void DeferredBuffer::End() {
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			Graphics::Texture::Clean(i, Graphics::ShaderStageList::PS);
		}
	}
	void DeferredBuffer::DebugRender() {
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*100.0f, 0.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
		}
	}


}