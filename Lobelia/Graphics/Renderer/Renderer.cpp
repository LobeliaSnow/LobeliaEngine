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
	//InstanceID SpriteRenderer::id = -1;
	void SpriteRenderer::Initialize() {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, false);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::LINEAR, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::BACK);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, false, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/ShaderFile/2D/VS.hlsl", "Main2D", Graphics::VertexShader::Model::VS_5_0, false);
		if (!ps)ps = std::make_shared<PixelShader>("Data/ShaderFile/2D/PS.hlsl", "Main2D", Graphics::PixelShader::Model::PS_5_0, true);
		ps->GetLinkage()->CreateInstance("TextureColor");
		ps->GetLinkage()->CreateInstance("VertexColor");
		ps->GetLinkage()->CreateInstance("InvertTextureColor");
		ps->GetLinkage()->CreateInstance("GrayscaleTextureColor");
		ps->GetLinkage()->CreateInstance("SepiaTextureColor");
		ps->SetLinkage(0);
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
	void SpriteRenderer::CutUV(const Math::Vector2& tex, const Math::Vector2& uv_pos, const Math::Vector2& uv_size) {
		//for (int index = 0; index < 4; index++) {
		mesh->GetBuffer()[1].tex.x = mesh->GetBuffer()[3].tex.x = (uv_pos.x + uv_size.x - 0.5f) / tex.x;
		mesh->GetBuffer()[0].tex.x = mesh->GetBuffer()[2].tex.x = (uv_pos.x + 0.5f) / tex.x;

		mesh->GetBuffer()[0].tex.y = mesh->GetBuffer()[1].tex.y = (uv_pos.y + 0.5f) / tex.y;
		mesh->GetBuffer()[2].tex.y = mesh->GetBuffer()[3].tex.y = (uv_pos.y + uv_size.y - 0.5f) / tex.y;
		//}
	}
	void SpriteRenderer::MeshTransform() {
		for (int index = 0; index < 4; index++) {
			mesh->GetBuffer()[index].pos = Trans2DPosition(vertex[index]);
		}
	}
	void SpriteRenderer::Render(Texture* tex, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color) {
		blend->Set(true);
		sampler->Set(true);
		rasterizer->Set(true);
		depthStencil->Set(true);
		inputLayout->Set();
		vs->Set();
		ps->Set();
		Device::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		PositionPlant(transform);
		if (transform.rotation != 0.0f)PositionRotation(transform);
		SetMeshColor(color);
		if (tex) {
			tex->Set(0, ShaderStageList::PS);
			CutUV(tex->GetSize(), uv_pos, uv_size);
		}
		else CutUV(uv_size - uv_pos, uv_pos, uv_size);
		MeshTransform();
		mesh->Set();
		Device::GetContext()->Draw(4, 0);
	}
	void SpriteRenderer::Render(Texture* tex, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color) {
		Transform2D trans = {};
		trans.position = pos;
		trans.scale = size;
		trans.rotation = rad;
		Render(tex, trans, uv_begin, uv_size, color);
	}
	void SpriteRenderer::Render(Texture* tex, Utility::Color color) {
		Transform2D trans = {};
		trans.position = {};
		trans.scale = View::nowSize;
		trans.rotation = 0.0f;
		Math::Vector2 size = trans.scale;
		if (tex)size = tex->GetSize();
		Render(tex, trans, Math::Vector2(0.0f, 0.0f), size, color);
	}
	void SpriteRenderer::Render(RenderTarget* rt, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color) {
		//Texture::Clean(0, ShaderStageList::PS);
		Render(rt->GetTexture(), transform, uv_pos, uv_size, color);
	}
	void SpriteRenderer::Render(RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color) {
		//Texture::Clean(0, ShaderStageList::PS);
		Render(rt->GetTexture(), pos, size, rad, uv_begin, uv_size, color);
	}
	void SpriteRenderer::Render(RenderTarget* rt, Utility::Color color) {
		//Texture::Clean(0, ShaderStageList::PS);
		Render(rt->GetTexture(), color);
	}
	void SpriteRenderer::CustumeRender(const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, const Math::Vector2& texture_size, Utility::Color color, bool state) {
		if (state) {
			blend->Set(true);
			sampler->Set(true);
			rasterizer->Set(true);
			depthStencil->Set(true);
			inputLayout->Set();
		}
		Device::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		Transform2D transform;
		transform.position = pos;
		transform.rotation = rad;
		transform.scale = size;
		PositionPlant(transform);
		if (transform.rotation != 0.0f)PositionRotation(transform);
		SetMeshColor(color);
		CutUV(texture_size, uv_begin, uv_size);
		MeshTransform();
		mesh->Set();
		Device::GetContext()->Draw(4, 0);
	}

	SpriteBatchRenderer::SpriteBatchRenderer(int render_limit) :RENDER_LIMIT(render_limit) {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, false);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::BACK);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, false, Graphics::StencilDesc(), false);
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
		for (int i = 0; i < TEXTURE_COUNT; i++) {
			textures[i] = nullptr;
		}
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

	Polygon3DRenderer::Polygon3DRenderer(int vertex_max) {
		if (!blend) blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, false);
		if (!sampler) sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::NONE);
		if (!depthStencil) depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, true, Graphics::StencilDesc(), false);
		if (!vs) vs = std::make_shared<VertexShader>("Data/Shaderfile/3D/VS.hlsl", "MainPolygon", Graphics::VertexShader::Model::VS_4_0);
		if (!ps) ps = std::make_shared<PixelShader>("Data/Shaderfile/3D/PS.hlsl", "MainPolygon", Graphics::PixelShader::Model::PS_5_0, true);
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		if (!inputLayout)inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());
		transform.CalcWorldMatrix();
		constantBuffer = std::make_unique<ConstantBuffer<DirectX::XMMATRIX>>(1, Config::GetRefPreference().systemCBActiveStage);
		mesh = std::make_unique<Mesh<Vertex>>(vertex_max);
		world = DirectX::XMMatrixIdentity();
		LoadTexture("");
	}
	Polygon3DRenderer::~Polygon3DRenderer() = default;
	void Polygon3DRenderer::SetTransformAndCalcMatrix(const Transform3D& transform) {
		DirectX::XMMATRIX translate, rotation, scalling;
		translate = DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
		rotation = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z);
		scalling = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z);
		scalling.m[0][0] *= -1;
		world = scalling;
		world *= rotation;
		//ここは少し審議が必要
		world *= translate;
	}
	void Polygon3DRenderer::ChangeTexture(Graphics::Texture* texture) { this->texture = texture; }
	void Polygon3DRenderer::LoadTexture(const char* file_path) { TextureFileAccessor::Load(file_path, &texture); }
	Polygon3DRenderer::Vertex* Polygon3DRenderer::Begin() { return mesh->GetBuffer(); }
	void Polygon3DRenderer::End() { mesh->Update(); }
	void Polygon3DRenderer::Render(int render_count, Topology topology) {
		Activate(); mesh->Set();
		Device::GetContext()->IASetPrimitiveTopology(s_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));
		texture->Set(0, ShaderStageList::PS);
		//DirectX::XMMATRIX world = {};
		//transform.GetWorldMatrixTranspose(&world);
		constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
		Device::GetContext()->Draw(render_count, 0);
	}
	Polygon2DRenderer::Polygon2DRenderer(int vertex_max) {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, false);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::LINEAR, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::BACK);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, false, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/ShaderFile/2D/VS.hlsl", "Main2D", Graphics::VertexShader::Model::VS_5_0, false);
		if (!ps)ps = std::make_shared<PixelShader>("Data/ShaderFile/2D/PS.hlsl", "Main2D", Graphics::PixelShader::Model::PS_5_0, true);
		ps->GetLinkage()->CreateInstance("TextureColor");
		ps->GetLinkage()->CreateInstance("VertexColor");
		ps->GetLinkage()->CreateInstance("InvertTextureColor");
		ps->GetLinkage()->CreateInstance("GrayscaleTextureColor");
		ps->GetLinkage()->CreateInstance("SepiaTextureColor");
		ps->SetLinkage(0);
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		if (!inputLayout)inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());
		mesh = std::make_unique<Mesh<Vertex>>(vertex_max);
		LoadTexture("");
	}
	Polygon2DRenderer::~Polygon2DRenderer() = default;
	void Polygon2DRenderer::ChangeTexture(Graphics::Texture* texture) { this->texture = texture; }
	void Polygon2DRenderer::LoadTexture(const char* file_path) { TextureFileAccessor::Load(file_path, &texture); }
	Polygon2DRenderer::Vertex* Polygon2DRenderer::Begin() { return mesh->GetBuffer(); }
	void Polygon2DRenderer::End() { mesh->Update(); }
	void Polygon2DRenderer::Render(int render_count, Topology topology) {
		Activate(); mesh->Set();
		Device::GetContext()->IASetPrimitiveTopology(s_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));
		texture->Set(0, ShaderStageList::PS);
		Device::GetContext()->Draw(render_count, 0);
	}
	void Primitive3D::CreateSphere(Sphere* sphere, int division) {
		if (division <= 0)STRICT_THROW("分割数の設定が正しく行えませんでした");
		//頂点作成
		std::vector<Math::Vector3> position((division + 1)*(division + 1));
		int posCount = 0;
		for (int i = 0; i <= division; i++) {
			float ph = PI * f_cast(i) / f_cast(division);
			float y = cosf(ph);
			float r = sinf(ph);
			for (int j = 0; j <= division; j++) {
				float th = 2.0f*PI*f_cast(j) / f_cast(division);
				float x = r * cosf(th);
				float z = r * sinf(th);
				position[posCount] = Math::Vector3(x, y, z);
				posCount++;
			}
		}
		//面作成
		sphere->vertexCount = division * division * 3 * 2;
		sphere->pos.resize(sphere->vertexCount);
		sphere->normal.resize(sphere->vertexCount);
		int faceCount = 0;
		for (int i = 0; i < division; i++) {
			for (int j = 0; j < division; j++) {
				int count = (division + 1)*i + j;
				Math::Vector4 normal;
				//上半分
				sphere->pos[faceCount].xyz = position[count];
				sphere->pos[faceCount + 1].xyz = position[count + 1];
				sphere->pos[faceCount + 2].xyz = position[count + division + 2];
				//法線
				normal = Math::CalcNormal(sphere->pos[faceCount], sphere->pos[faceCount + 1], sphere->pos[faceCount + 2]);
				for (int k = 0; k < 3; k++) {
					sphere->normal[faceCount + k] = normal;
				}
				//w成分を1.0に
				sphere->pos[faceCount].w = 1.0f;
				sphere->pos[faceCount + 1].w = 1.0f;
				sphere->pos[faceCount + 2].w = 1.0f;
				faceCount += 3;
				//下半分
				sphere->pos[faceCount].xyz = position[count];
				sphere->pos[faceCount + 1].xyz = position[count + division + 2];
				sphere->pos[faceCount + 2].xyz = position[count + division + 1];
				normal = Math::CalcNormal(sphere->pos[faceCount], sphere->pos[faceCount + 1], sphere->pos[faceCount + 2]);
				//法線
				for (int k = 0; k < 3; k++) {
					sphere->normal[faceCount + k] = normal;
				}
				//w成分を1.0に
				sphere->pos[faceCount].w = 1.0f;
				sphere->pos[faceCount + 1].w = 1.0f;
				sphere->pos[faceCount + 2].w = 1.0f;
				faceCount += 3;
			}
		}
	}

	DebugRenderer::DebugRenderer()
#ifdef _DEBUG
		:verticesLine(nullptr), verticesPoint(nullptr), verticeSphere(nullptr)
#endif
	{}
	void DebugRenderer::Initialize() {
#ifdef _DEBUG
		line = std::make_unique<Graphics::Polygon3DRenderer>(10240);
		line->LoadTexture("");
		point = std::make_unique<Graphics::Polygon3DRenderer>(512);
		point->LoadTexture("");
		sphere = std::make_unique<Graphics::Polygon3DRenderer>(25600);
		sphere->LoadTexture("");
#endif
	}
	void DebugRenderer::Begin() {
#ifdef _DEBUG
		verticesLine = line->Begin();
		lineCount = 0;
		verticesPoint = point->Begin();
		pointCount = 0;
		verticeSphere = sphere->Begin();
		sphereVertexCount = 0;
#endif
	}
	void DebugRenderer::SetLine(const Math::Vector3& p0, const Math::Vector3&p1, Utility::Color color) {
#ifdef _DEBUG
		if (!verticesLine)STRICT_THROW("Beginを行ってください");
		int line0Index = lineCount * 2;
		verticesLine[line0Index].pos.xyz = p0; verticesLine[line0Index].pos.w = 1.0f;
		verticesLine[line0Index].color = Math::Vector4(color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB(), color.GetNormalizedA());
		verticesLine[line0Index].normal = Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		int line1Index = lineCount * 2 + 1;
		verticesLine[line1Index].pos.xyz = p1; verticesLine[line1Index].pos.w = 1.0f;
		verticesLine[line1Index].color = Math::Vector4(color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB(), color.GetNormalizedA());
		verticesLine[line1Index].normal = Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		lineCount++;
#endif
	}
	void DebugRenderer::SetPoint(const Math::Vector3& p, Utility::Color color) {
#ifdef _DEBUG
		if (!verticesPoint)STRICT_THROW("Beginを行ってください");
		verticesPoint[pointCount].pos.xyz = p; verticesPoint[pointCount].pos.w = 1.0f;
		verticesPoint[pointCount].color = Math::Vector4(color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB(), color.GetNormalizedA());
		verticesPoint[pointCount].normal = Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		pointCount++;
#endif
	}
	void DebugRenderer::SetSphere(const Math::Vector3& p, float radius, int division, Utility::Color color) {
#ifdef _DEBUG
		Primitive3D::Sphere sphere;
		Primitive3D::CreateSphere(&sphere, division);
		SetSphere(p, radius, sphere, color);
#endif
	}
	void DebugRenderer::SetSphere(const Math::Vector3& pos, float radius, const Primitive3D::Sphere& sphere, Utility::Color color) {
#ifdef _DEBUG
		Math::Vector4 vColor(color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB(), color.GetNormalizedA());
		for (int i = 0; i < sphere.vertexCount; i++) {
			//位置
			verticeSphere[sphereVertexCount + i].pos = sphere.pos[i];
			verticeSphere[sphereVertexCount + i].pos.xyz *= radius;
			verticeSphere[sphereVertexCount + i].pos.xyz += pos;
			//法線
			verticeSphere[sphereVertexCount + i].normal = sphere.normal[i];
			//色
			verticeSphere[sphereVertexCount + i].color = vColor;
		}
		sphereVertexCount += sphere.vertexCount;
#endif
	}
	void DebugRenderer::End() {
#ifdef _DEBUG
		line->End(); verticesLine = nullptr;
		point->End(); verticesPoint = nullptr;
		sphere->End(); verticeSphere = nullptr;
#endif
	}
	void DebugRenderer::Render() {
#ifdef _DEBUG
		if (lineCount != 0)line->Render(lineCount * 2, Topology::LINE_LIST);
		if (pointCount != 0)point->Render(pointCount, Topology::POINT_LIST);
		if (sphereVertexCount != 0)sphere->Render(sphereVertexCount, Topology::TRI_LIST);
#endif
	}
}