#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/RenderState/RenderState.hpp"
#include "Graphics/RenderableObject/RenderableObject.hpp"
#include "Graphics/Renderer/Renderer.hpp"
#include "Graphics/Sprite/Sprite.hpp"
#include	 "Graphics/View/View.hpp"
#include "Config/Config.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	std::unique_ptr<Mesh<SpriteRenderer::Vertex>> SpriteRenderer::mesh;
	//こいつが実際のスクリーンスペースの座標を所持している
	Math::Vector2 SpriteRenderer::vertex[4] = { { -1,-1 },{ -1,0 },{ 0,-1 },{ 0,0 } };
	InstanceID SpriteRenderer::id = -1;
	void SpriteRenderer::Initialize() {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BlendPreset::COPY, true, false);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SamplerPreset::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RasterizerPreset::BACK);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DepthPreset::ALWAYS, false, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/ShaderFile/2D/VS.hlsl", "Main2D", Graphics::VertexShader::Model::VS_5_0, false);
		if (!ps)ps = std::make_shared<PixelShader>("Data/ShaderFile/2D/PS.hlsl", "Main2D", Graphics::PixelShader::Model::PS_5_0, true);
		id = ps->GetLinkage()->CreateInstance("TextureColor");
		ps->GetLinkage()->CreateInstance("VertexColor");
		ps->GetLinkage()->CreateInstance("InvertTextureColor");
		ps->GetLinkage()->CreateInstance("GrayscaleTextureColor");
		ps->GetLinkage()->CreateInstance("SepiaTextureColor");

		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		if (!inputLayout)inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());
		mesh = std::make_unique<Mesh<Vertex>>(4);
	}
	Math::Vector4 SpriteRenderer::Trans2DPosition(Math::Vector2 pos) {
		//いったん仮でウインドウサイズ
		//ここちょっと工夫いる。中間のデータを扱う何かを作るほうが良い
		Math::Vector2 w = View::nowSize;
		return Math::Vector4(static_cast<float>((pos.x - w.x / 2) / (w.x / 2)), static_cast<float>((pos.y - w.y / 2) / (-w.y / 2)), 0.0f, 1.0f);
	}
	void SpriteRenderer::PositionPlant(const Transform2D& transform) {
		vertex[0].x = static_cast<float>(transform.position.x);								vertex[0].y = static_cast<float>(transform.position.y);
		vertex[1].x = static_cast<float>(transform.position.x + transform.scale.x);		vertex[1].y = static_cast<float>(transform.position.y);
		vertex[3].x = static_cast<float>(transform.position.x + transform.scale.x);		vertex[3].y = static_cast<float>(transform.position.y + transform.scale.y);
		vertex[2].x = static_cast<float>(transform.position.x);								vertex[2].y = static_cast<float>(transform.position.y + transform.scale.y);
	}
	void SpriteRenderer::PositionRotation(const Transform2D& transform) {
		for (int index = 0; index < 4; index++) {
			Math::Vector2 tmp = {};
			tmp.x = vertex[index].x - transform.position.x - transform.scale.x / 2;
			tmp.y = vertex[index].y - transform.position.y - transform.scale.y / 2;
			vertex[index].x = tmp.x*cosf(transform.rotation) - tmp.y*sinf(transform.rotation);
			vertex[index].y = tmp.x*sinf(transform.rotation) + tmp.y*cosf(transform.rotation);
			vertex[index].x += transform.position.x + transform.scale.x / 2;
			vertex[index].y += transform.position.y + transform.scale.y / 2;
		}
	}
	void SpriteRenderer::SetMeshColor(Utility::Color color) {
		for (int index = 0; index < 4; index++) {
			mesh->GetBuffer()[index].col.x = color.GetNormalizedR();
			mesh->GetBuffer()[index].col.y = color.GetNormalizedG();
			mesh->GetBuffer()[index].col.z = color.GetNormalizedB();
			mesh->GetBuffer()[index].col.w = color.GetNormalizedA();
		}
	}
	void SpriteRenderer::CutUV(Texture* tex, const Math::Vector2& uv_pos, const Math::Vector2& uv_size) {
		for (int index = 0; index < 4; index++) {
			mesh->GetBuffer()[1].tex.x = mesh->GetBuffer()[3].tex.x = (uv_pos.x + uv_size.x - 0.5f) / tex->GetSize().x;
			mesh->GetBuffer()[0].tex.x = mesh->GetBuffer()[2].tex.x = (uv_pos.x + 0.5f) / tex->GetSize().x;

			mesh->GetBuffer()[0].tex.y = mesh->GetBuffer()[1].tex.y = (uv_pos.y + 0.5f) / tex->GetSize().y;
			mesh->GetBuffer()[2].tex.y = mesh->GetBuffer()[3].tex.y = (uv_pos.y + uv_size.y - 0.5f) / tex->GetSize().y;
		}
	}
	void SpriteRenderer::MeshTransform() {
		for (int index = 0; index < 4; index++) {
			mesh->GetBuffer()[index].pos = Trans2DPosition(vertex[index]);
		}
	}
	void SpriteRenderer::Render(Texture* tex, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color) {
		blend->Set();
		sampler->Set();
		rasterizer->Set();
		depthStencil->Set();
		inputLayout->Set();
		vs->Set();
		ps->Set(1, &id);
		tex->Set(0, ShaderStageList::PS);
		Device::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		PositionPlant(transform);
		if (transform.rotation != 0.0f)PositionRotation(transform);
		SetMeshColor(color);
		CutUV(tex, uv_pos, uv_size);
		MeshTransform();
		mesh->Set();
		Device::GetContext()->Draw(4, 0);
	}
	void SpriteRenderer::Render(Texture* tex, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color) {
		Transform2D trans = {};
		trans.position = pos;
		trans.scale = size;
		trans.rotation = rad;
		Render(tex, trans, uv_begin, uv_end, color);
	}
	void SpriteRenderer::Render(Texture* tex, Utility::Color color) {
		Transform2D trans = {};
		trans.position = {};
		trans.scale = View::nowSize;
		trans.rotation = 0.0f;
		Render(tex, trans, Math::Vector2(0.0f, 0.0f), tex->GetSize(), color);
	}
	void SpriteRenderer::Render(RenderTarget* rt, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color) {
		Texture::Clean(0, ShaderStageList::PS);
		Render(rt->GetTexture(), transform, uv_pos, uv_size, color);
	}
	void SpriteRenderer::Render(RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color) {
		Texture::Clean(0, ShaderStageList::PS);
		Render(rt->GetTexture(), pos, size, rad, uv_begin, uv_end, color);
	}
	void SpriteRenderer::Render(RenderTarget* rt, Utility::Color color) {
		Texture::Clean(0, ShaderStageList::PS);
		Render(rt->GetTexture(), color);
	}
	
	SpriteBatchRenderer::SpriteBatchRenderer(int render_limit) :RENDER_LIMIT(render_limit) {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BlendPreset::COPY, true, false);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SamplerPreset::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RasterizerPreset::BACK);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DepthPreset::ALWAYS, false, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/ShaderFile/2D/VS.hlsl", "Main2DInstRenderer", Graphics::VertexShader::Model::VS_5_0, false);
		if (!ps)ps = std::make_shared<PixelShader>("Data/ShaderFile/2D/PS.hlsl", "Main2DRenderer", Graphics::PixelShader::Model::PS_5_0, false);
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		if (!inputLayout)inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());
		BufferCreator::Create(instanceBuffer.GetAddressOf(), nullptr, sizeof(Instance)*render_limit, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, sizeof(float));
		mesh = std::make_unique<Mesh<Vertex>>(4);
		mesh->GetBuffer()[0] = { { -1.0f,  +1.0f, +0.0f, +1.0f },{ 0.0f,1.0f } };
		mesh->GetBuffer()[1] = { { -1.0f,  -1.0f,  +0.0f, +1.0f },{ 0.0f,0.0f } };
		mesh->GetBuffer()[2] = { { +1.0f, +1.0f, +0.0f, +1.0f },{ 1.0f,1.0f } };
		mesh->GetBuffer()[3] = { { +1.0f, -1.0f, +0.0f, +1.0f },{ 1.0f,0.0f } };
		mesh->Update();
	}
	const Texture* SpriteBatchRenderer::GetTexture(int index) {
		if (s_cast<UINT>(index) >= TEXTURE_COUNT)STRICT_THROW("slotが限界値を超えています");
		return textures[index];
	}
	void SpriteBatchRenderer::SetTexture(int slot, Texture* texture) {
		if (s_cast<UINT>(slot) >= TEXTURE_COUNT)STRICT_THROW("slotが限界値を超えています");
		textures[slot] = texture;
	}
	void SpriteBatchRenderer::Begin() {
		D3D11_MAPPED_SUBRESOURCE mapResource = {};
		HRESULT hr = S_OK;
		hr = Device::GetContext()->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapResource);
		instances = static_cast<Instance*>(mapResource.pData);
		renderCount = 0;
	}
	void SpriteBatchRenderer::Set(int slot, const Transform2D& transform, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, /*float rotate_axis_x, float rotate_axis_y, */Utility::Color color) {
		if (s_cast<UINT>(slot) >= s_cast<UINT>(TEXTURE_COUNT) || !textures[slot])STRICT_THROW("存在しないテクスチャです");
		Math::Vector2 view(2.0f / View::nowSize.x, -2.0f / View::nowSize.y);
		Math::Vector2 rotate(cosf(transform.rotation), sinf(transform.rotation));
		instances[renderCount].ndc._11 = view.x*(transform.scale.x / 2)*rotate.x;
		instances[renderCount].ndc._21 = view.y*(transform.scale.x / 2)*rotate.y;
		instances[renderCount].ndc._31 = 0.0f;
		instances[renderCount].ndc._41 = 0.0f;
		instances[renderCount].ndc._12 = view.x*(transform.scale.y / 2)*-rotate.y;
		instances[renderCount].ndc._22 = view.y*(transform.scale.y / 2)*rotate.x;
		instances[renderCount].ndc._32 = 0.0f;
		instances[renderCount].ndc._42 = 0.0f;
		instances[renderCount].ndc._13 = 0.0f;
		instances[renderCount].ndc._23 = 0.0f;
		instances[renderCount].ndc._33 = 1.0f;
		instances[renderCount].ndc._43 = 0.0f;
		instances[renderCount].ndc._14 = view.x*(/*-rotate_axis_x * rotate.x + -rotate_axis_y * -rotate.y + rotate_axis_x + */transform.position.x + transform.scale.x / 2) - 1.0f;
		instances[renderCount].ndc._24 = view.y*(/*-rotate_axis_x * rotate.y + -rotate_axis_y * rotate.x + rotate_axis_y +*/ transform.position.y + transform.scale.y / 2) + 1.0f;
		instances[renderCount].ndc._34 = 0.0f;
		instances[renderCount].ndc._44 = 1.0f;

		instances[renderCount].col.x = color.GetNormalizedR();
		instances[renderCount].col.y = color.GetNormalizedG();
		instances[renderCount].col.z = color.GetNormalizedB();
		instances[renderCount].col.w = color.GetNormalizedA();
		Math::Vector2 size = textures[slot]->GetSize();
		instances[renderCount].uvTrans.x = static_cast<float>(uv_begin.x) / static_cast<float>(size.x);
		instances[renderCount].uvTrans.y = static_cast<float>(uv_begin.y) / static_cast<float>(size.y);
		instances[renderCount].uvTrans.z = static_cast<float>(uv_size.x) / static_cast<float>(size.x);
		instances[renderCount].uvTrans.w = static_cast<float>(uv_size.y) / static_cast<float>(size.y);

		instances[renderCount].textureSlot = slot;

		renderCount++;
	}
	void SpriteBatchRenderer::Set(int slot, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color) {
		Transform2D trans = {};
		trans.position = pos;
		trans.scale = size;
		trans.rotation = rad;
		Set(slot, trans, uv_begin, uv_size, color);
	}
	void SpriteBatchRenderer::Set(int slot, Utility::Color color) {
		Transform2D trans = {};
		trans.position = {};
		trans.scale = View::nowSize;
		trans.rotation = 0.0f;
		Set(slot, trans, Math::Vector2(0.0f, 0.0f), textures[slot]->GetSize(), color);
	}
	void SpriteBatchRenderer::End() {
		Device::GetContext()->Unmap(instanceBuffer.Get(), 0);
		instances = nullptr;
	}
	void SpriteBatchRenderer::Render() {
		Activate();
		for (int i = 0; i < TEXTURE_COUNT; i++) {
			if (textures[i])textures[i]->Set(i + 20, ShaderStageList::PS);
		}
		Device::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ID3D11Buffer* buffer[2] = { mesh->GetVertexBuffer().Get(),instanceBuffer.Get() };
		UINT strides[2] = { sizeof(Vertex), sizeof(Instance) };
		UINT offset[2] = { 0,0 };
		Device::GetContext()->IASetVertexBuffers(0, 2, buffer, strides, offset);
		Device::GetContext()->DrawInstanced(4, renderCount, 0, 0);
	}
}