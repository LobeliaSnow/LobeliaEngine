#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/Sprite/Sprite.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/RenderState/RenderState.hpp"
//本当はいらない
#include "Scene/Scene.hpp"
#include "Application/Application.hpp"
#include "Graphics/View/View.hpp"
#include "Graphics/Pipeline/Pipeline.hpp"
#include "Graphics/RenderableObject/RenderableObject.hpp"
#include "Graphics/Renderer/Renderer.hpp"

namespace Lobelia::Graphics {
	//Spriteなので頂点数4は変わらないと考えて直値
	Sprite::Sprite(const char* file_path) {
		TextureFileAccessor::Load(file_path, &texture);
	}
	Sprite::~Sprite() = default;
	Texture* Sprite::GetTexture() { return texture; }
	void Sprite::Render(const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color, bool set_default_pipeline) {
		SpriteRenderer::Render(texture, transform, uv_pos, uv_size, color, set_default_pipeline);
	}
	void Sprite::Render(const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color, bool default) {
		SpriteRenderer::Render(texture, pos, size, rad, uv_begin, uv_end, color, default);
	}
	void Sprite::Render(Utility::Color color, bool set_default_pipeline) {
		SpriteRenderer::Render(texture, color, set_default_pipeline);
	}

	SpriteBatch::SpriteBatch(const char* file_path, int render_limit) :mesh(std::make_unique<Mesh<Vertex>>(4)), RENDER_LIMIT(render_limit), renderCount(0) {
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(ResourceBank<VertexShader>::Get(D_VS2D_BATCH));
		material = std::make_unique<Material>("Sprite Batch Material", file_path);
		inputLayout = std::make_unique<InputLayout>(ResourceBank<VertexShader>::Get(D_VS2D_BATCH), reflector.get());
		BufferCreator::Create(instanceBuffer.GetAddressOf(), nullptr, sizeof(Instance)*RENDER_LIMIT, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, sizeof(float));
		mesh->GetBuffer()[0] = { { -1.0f,  +1.0f, +0.0f, +1.0f },{ 0.0f,1.0f } };
		mesh->GetBuffer()[1] = { { -1.0f,  -1.0f,  +0.0f, +1.0f },{ 0.0f,0.0f } };
		mesh->GetBuffer()[2] = { { +1.0f, +1.0f, +0.0f, +1.0f },{ 1.0f,1.0f } };
		mesh->GetBuffer()[3] = { { +1.0f, -1.0f, +0.0f, +1.0f }, { 1.0f,0.0f } };
		mesh->Update();
	}
	SpriteBatch::~SpriteBatch() = default;
	Material* SpriteBatch::GetMaterial() { return material.get(); }

	void SpriteBatch::BeginRender() {
		D3D11_MAPPED_SUBRESOURCE mapResource = {};
		HRESULT hr = S_OK;
		hr = Device::GetContext()->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapResource);
		instances = static_cast<Instance*>(mapResource.pData);
	}
	void SpriteBatch::Render(const Transform2D& transform, const Math::Vector2& upos, const Math::Vector2& usize, float rotate_axis_x, float rotate_axis_y, Utility::Color color) {
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
		instances[renderCount].ndc._14 = view.x*(-rotate_axis_x * rotate.x + -rotate_axis_y * -rotate.y + rotate_axis_x + transform.position.x + transform.scale.x / 2) - 1.0f;
		instances[renderCount].ndc._24 = view.y*(-rotate_axis_x * rotate.y + -rotate_axis_y * rotate.x + rotate_axis_y + transform.position.y + transform.scale.y / 2) + 1.0f;
		instances[renderCount].ndc._34 = 0.0f;
		instances[renderCount].ndc._44 = 1.0f;

		instances[renderCount].col.x = color.GetNormalizedR();
		instances[renderCount].col.y = color.GetNormalizedG();
		instances[renderCount].col.z = color.GetNormalizedB();
		instances[renderCount].col.w = color.GetNormalizedA();

		Math::Vector2 size = material->GetTexture()->GetSize();
		instances[renderCount].uvTrans.x = static_cast<float>(upos.x) / static_cast<float>(size.x);
		instances[renderCount].uvTrans.y = static_cast<float>(upos.y) / static_cast<float>(size.y);
		instances[renderCount].uvTrans.z = static_cast<float>(usize.x) / static_cast<float>(size.x);
		instances[renderCount].uvTrans.w = static_cast<float>(usize.y) / static_cast<float>(size.y);
		renderCount++;
	}
	void SpriteBatch::EndRender(bool set_default_pipeline) {
		Device::GetContext()->Unmap(instanceBuffer.Get(), 0);
		if (set_default_pipeline)Graphics::PipelineManager::PipelineGet(D_PIPE2D_BATCH)->Activate(true);
		//Graphics::RenderState::DepthSet("On");
		inputLayout->Set(); material->Set();
		Device::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ID3D11Buffer* buffer[2] = { mesh->GetVertexBuffer().Get(),instanceBuffer.Get() };
		UINT strides[2] = { sizeof(Vertex), sizeof(Instance) };
		UINT offset[2] = { 0,0 };
		Device::GetContext()->IASetVertexBuffers(0, 2, buffer, strides, offset);
		Device::GetContext()->DrawInstanced(4, renderCount, 0, 0);
		renderCount = 0;
	}
}