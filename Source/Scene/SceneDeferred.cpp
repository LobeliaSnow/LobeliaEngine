#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//Screen Space Motion Blurやってみたい
//被写界深度やってみたい

//スキンメッシュのキャラを歩かせて、当たり判定(Raypick)をGPGPUでとるのも面白いと思う
//歩かせるのはActorクラス使えばすぐ

//TODO : SSAOPSの可変解像度対応(ビューを作ってやるだけ？)
//TODO : シェーダーの整理

//テストシーン
//最終的にここでの経験と結果を用いてレンダリングエンジンを作る予定ではいるが、就活終わった後になると思われる。

//現状実装されてるもの一覧(このシーンはディファードシェーディングです)
//Define.hのスイッチで一部の機能のスイッチが可能
//ハーフランバート
//線形フォグ
//法線マップ
//複数ポイントライト
//SSAOPS 遅い
//SSAOCS PSより早い
//ガウスフィルタPS
//ガウスフィルタCS 現状遅い 最適化不足
//シャドウマップ
//バリアンスシャドウマップ
//カスケードシャドウマップ
//カスケードバリアンスシャドウマップ(カスケードシャドウマップと、バリアンスシャドウマップの合わせ技)

namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 127;
	}
	//---------------------------------------------------------------------------------------------
	//
	//	Skybox
	//
	//---------------------------------------------------------------------------------------------
	SkyBox::SkyBox(const char* model_path, const char* mt_path) {
		model = std::make_unique<Graphics::Model>(model_path, mt_path);
		//DepthStencilState(DEPTH_PRESET preset, bool depth, StencilDesc sdesc, bool stencil);
		depth = std::make_shared<Graphics::DepthStencilState>(Graphics::DEPTH_PRESET::LESS, false, Graphics::StencilDesc(), false);
		model->Scalling(30.0f);
	}
	void SkyBox::Render(Camera* camera) {
		auto defaultDepth = Graphics::Model::GetDepthStencilState();
		DirectX::XMVECTOR temp = {};
		DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(&temp, camera->GetView()->GetRowViewMatrix());
		model->GetPixelShader()->SetLinkage(1);
		model->Translation(Math::Vector3(inv._41, inv._42, inv._43));
		model->CalcWorldMatrix();
		Graphics::Model::ChangeDepthStencilState(depth);
		model->Render();
		model->GetPixelShader()->SetLinkage(0);
		Graphics::Model::ChangeDepthStencilState(defaultDepth);
	}
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
		//さらに速度重視なら、精度はやばいがこれもありDXGI_FORMAT_R11G11B10_FLOAT
		rts[i_cast(BUFFER_TYPE::NORMAL)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		rts[i_cast(BUFFER_TYPE::COLOR)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_SNORM);
		rts[i_cast(BUFFER_TYPE::VIEW_POS)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		rts[i_cast(BUFFER_TYPE::SHADOW)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16_FLOAT);
		//VertexShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage = false);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateGBufferVS", Graphics::VertexShader::Model::VS_4_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateGBufferPS", Graphics::PixelShader::Model::PS_4_0);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(8, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
	}
	std::shared_ptr<Graphics::RenderTarget> DeferredBuffer::GetRenderTarget(BUFFER_TYPE type) { return rts[i_cast(type)]; }
	void DeferredBuffer::AddModel(std::shared_ptr<Graphics::Model> model, bool use_normal_map) {
		models.push_back(ModelStorage{ model,i_cast(use_normal_map),FALSE });
	}
	void DeferredBuffer::RenderGBuffer() {
		End();
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			rts[i]->Clear(0x00000000);
		}
		Graphics::RenderTarget::Activate(rts[0].get(), rts[1].get(), rts[2].get(), rts[3].get(), rts[4].get());
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		for (auto&& weak : models) {
			if (weak.model.expired())continue;
			std::shared_ptr<Graphics::Model> model = weak.model.lock();
			cbuffer->Activate(weak.info);
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
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
		}
	}

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
	PointLightDeferred::PointLightDeferred() : DeferredShader("Data/ShaderFile/3D/deferred.hlsl", "", "FullDeferredPS") {
		cbuffer = std::make_unique<Graphics::ConstantBuffer<PointLights>>(6, Graphics::ShaderStageList::PS);
	}
	void PointLightDeferred::SetLightBuffer(int index, const PointLight& p_light) {
		lights.pos[index] = p_light.pos;
		lights.color[index] = p_light.color;
		lights.attenuation[index].x = p_light.attenuation;
	}
	void PointLightDeferred::SetUseCount(int use_count) { lights.usedLightCount = use_count; }
	void PointLightDeferred::Update() {
		cbuffer->Activate(lights);
	}
	//---------------------------------------------------------------------------------------------
	ShadowBuffer::ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance) :size(size), count(split_count) {
		rts.resize(split_count); views.resize(split_count);
		//分割する際のnear/farを持つ、+1の理由は、最初のnearから始まって次のfarだけを記録していくが
		//最初のfarだけはnearとして共有されないので、その分の+1
		cascadeValues.resize(split_count + 1);
		splitPositions.resize(split_count);
		gaussian.resize(split_count);
		DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;
		Math::Vector2 ssize = size;
		for (int i = 0; i < split_count; i++) {
			rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
			views[i] = std::make_unique<Graphics::View>(Math::Vector2(), size, PI / 4.0f, 50, 400.0f);
#ifdef GAUSSIAN_CS
			gaussian[i] = std::make_unique<GaussianFilterCS>(size, format);
#endif
#ifdef GAUSSIAN_PS
			gaussian[i] = std::make_unique<GaussianFilterPS>(size, format);
#endif
			gaussian[i]->SetDispersion(0.01f);
		}
		//sampler = std::make_unique<Graphics::SamplerState>(Graphics::SAMPLER_PRESET::COMPARISON_LINEAR, 16);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapVS", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(10, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		info.useShadowMap = TRUE; info.useVariance = i_cast(use_variance);
		//縮小バッファにして処理稼ぐのもいいかも
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use shadow", &info.useShadowMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use variance", &info.useVariance, false);
#endif
	}
	void ShadowBuffer::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void ShadowBuffer::SetTarget(const Math::Vector3& at) { this->at = at; }
	//ちょっと大丈夫かは不明、ダメそうなら調整します
	void ShadowBuffer::ComputeSplit(float lamda, float near_z, float far_z) {
#ifdef CASCADE
		//通常のシャドウマップ
		if (count == 1) {
			cascadeValues[0] = near_z;
			cascadeValues[1] = far_z;
			return;
		}
		//カスケード
		float invM = 1.0f / f_cast(count);
		float farDivisionNear = far_z / near_z;
		float farSubNear = far_z - near_z;
		//実用分割スキームを適用
		// ※ GPU Gems 3, Chapter 10. Parallel-Split Shadow Maps on Programmable GPUs.
		//    http://http.developer.nvidia.com/GPUGems3/gpugems3_ch10.html を参照.
		for (int i = 1; i < count + 1; i++) {
			//対数分割スキーム
			float log = near_z * powf(farDivisionNear, invM*i);
			//一様分割スキーム
			float uni = near_z + farSubNear * i*invM;
			//上記の2つの演算結果を線形補間する
			cascadeValues[i] = lamda * log + uni * (1.0f - lamda);
		}
		cascadeValues[0] = near_z;
		cascadeValues[count] = far_z;
		for (int i = 1; i < count + 1; i++) {
			info.splitPos[i - 1] = cascadeValues[i];
		}
#endif
	}
	void ShadowBuffer::CameraUpdate() {
		Math::Vector3 front = at - pos; front.Normalize();
		up = Math::Vector3(0.01f, 1.0f, 0.01f); up.Normalize();
		Math::Vector3 right = Math::Vector3::Cross(up, front); right.Normalize();
		up = Math::Vector3::Cross(front, right);
#ifdef CASCADE
		info.pos.x = pos.x; info.pos.y = pos.y; info.pos.z = pos.z; info.pos.w = 1.0f;
		info.front.x = front.x; info.front.y = front.y; info.front.z = front.z; info.front.w = 0.0f;
		float nearZ = 1.0f;
		float farZ = 1000.0f;
		ComputeSplit(0.5f, nearZ, farZ);
#endif
		for (int i = 0; i < count; i++) {
			views[i]->SetEyePos(pos);
			views[i]->SetEyeTarget(at);
			views[i]->SetEyeUpDirection(up);
#ifdef CASCADE
			views[i]->SetNear(cascadeValues[i]);
			views[i]->SetFar(cascadeValues[count]);
#endif
		}
	}
	void ShadowBuffer::AddModel(std::shared_ptr<Graphics::Model> model) { models.push_back(model); }
	void ShadowBuffer::CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt) {
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_0) == 1)info.useShadowMap = !info.useShadowMap;
		if (Input::GetKeyboardKey(DIK_9) == 1)info.useVariance = !info.useVariance;
#endif
		if (!info.useShadowMap) {
			models.clear();
			return;
		}
		CameraUpdate();
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		for (int i = 0; i < count; i++) {
			views[i]->Activate();
			rts[i]->Clear(0x00000000);
			rts[i]->Activate();
			for (auto&& weak : models) {
				if (weak.expired())continue;
				auto model = weak.lock();
				model->Render();
			}
		}
		models.clear();
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		active_view->Activate();
		active_rt->Activate();
		//ガウスによるぼかし バリアンス用
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->Dispatch(active_view, active_rt, rts[i]->GetTexture());
			}
		}
	}
	void ShadowBuffer::Begin() {
		//情報の更新
		DirectX::XMStoreFloat4x4(&info.view, views[0]->GetColumnViewMatrix());
		for (int i = 0; i < count; i++) {
			DirectX::XMStoreFloat4x4(&info.proj[i], views[i]->GetColumnProjectionMatrix());
			if (info.useVariance)gaussian[i]->Begin(6 + i);
			else rts[i]->GetTexture()->Set(6 + i, Graphics::ShaderStageList::PS);
		}
		cbuffer->Activate(info);
	}
	void ShadowBuffer::End() {
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->End();
			}
		}
		else {
			for (int i = 0; i < count; i++) {
				Graphics::Texture::Clean(6 + i, Graphics::ShaderStageList::PS);
			}
		}
	}
	void ShadowBuffer::DebugRender() {
		for (int i = 0; i < count; i++) {
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*200.0f, 200.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
			gaussian[i]->DebugRender(Math::Vector2(i*200.0f, 400.0f), Math::Vector2(200.0f, 200.0f));
		}
	}
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
	UnorderedAccessView::UnorderedAccessView(Graphics::Texture* texture) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(texture->Get().Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAVの作成に失敗");
	}
	UnorderedAccessView::UnorderedAccessView(StructuredBuffer* structured_buffer) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(structured_buffer->buffer.Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAVの作成に失敗");
	}
	void UnorderedAccessView::Set(int slot) {
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, uav.GetAddressOf(), nullptr);
	}
	void UnorderedAccessView::Clean(int slot) {
		ID3D11UnorderedAccessView* null = nullptr;
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, &null, nullptr);
	}
	//---------------------------------------------------------------------------------------------
	StructuredBuffer::StructuredBuffer(int struct_size, int count) {
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = struct_size * count;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = struct_size;
		//D3D11_SUBRESOURCE_DATA initData = {};
		//initData.pSysMem = 
		Graphics::Device::Get()->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
		//SRV（シェーダーリソースビュー）作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srdesc = {};
		srdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srdesc.Format = DXGI_FORMAT_UNKNOWN;
		srdesc.BufferEx.NumElements = count;
		Graphics::Device::Get()->CreateShaderResourceView(buffer.Get(), &srdesc, srv.GetAddressOf());
	}

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
		cs->Run(i_cast(size.x + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, i_cast(size.y + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, 1);
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
		csX->Run(i_cast((size.x + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), i_cast((size.y + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), 1);
		uavPass1->Clean(0);
		uavPass2->Set(0);
		rwTexturePass1->Set(0, Graphics::ShaderStageList::CS);
		csY->Run(i_cast((size.x + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), i_cast((size.y + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), 1);
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
	//---------------------------------------------------------------------------------------------
	//
	//	Raycaster
	//
	//---------------------------------------------------------------------------------------------
	Raycaster::Raycaster() {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/GPGPU/GPGPU.hlsl", "RaycastCS");
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Camera
	//
	//---------------------------------------------------------------------------------------------
	Camera::Camera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at) :pos(pos), at(at) {
		//複数のビューを使わない前提 使いたい場合は引数でとる
		view = std::make_shared<Graphics::View>(Math::Vector2(), scale);
		up = TakeUp();
	}
	void Camera::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void Camera::SetTarget(const Math::Vector3& at) { this->at = at; }
	std::shared_ptr<Graphics::View> Camera::GetView() { return view; }
	Math::Vector3 Camera::TakeFront() {
		Math::Vector3 ret = at - pos; ret.Normalize();
		return ret;
	}
	Math::Vector3 Camera::TakeRight() {
		Math::Vector3 front = TakeFront();
		Math::Vector3 tempUP(0.001f, 1.0f, 0.001f); tempUP.Normalize();
		Math::Vector3 ret = Math::Vector3::Cross(tempUP, front); /*ret.Normalize();*/
		return ret;
	}
	Math::Vector3 Camera::TakeUp() {
		Math::Vector3 tempFront = TakeFront();
		Math::Vector3 tempRight = TakeRight();
		Math::Vector3 ret = Math::Vector3::Cross(tempFront, tempRight); /*ret.Normalize();*/
		return ret;
	}
	void Camera::Activate() {
		view->SetEyePos(pos);
		view->SetEyeTarget(at);
		view->SetEyeUpDirection(up);
		view->Activate();
	}
	DebugCamera::DebugCamera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at) :Camera(scale, pos, at) {
		radius = (pos - at).Length();
		front = TakeFront();
		right = TakeRight();
	}
	void DebugCamera::Update() {
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		Math::Vector3 rightMove = {};
		Math::Vector3 upMove = {};
		//直径
		float circumference = radius * PI;
		//ちょうどいい感じに動く割合算出
		circumference /= 600.0f;
		//マウス
		float wheel = Input::Mouse::GetInstance()->GetWheel();
		//注視点までの距離
		Math::Vector2 mmove = Input::Mouse::GetInstance()->GetMove();
		rightMove += right * mmove.x * circumference;
		upMove += up * mmove.y * circumference;
		//キーボード
		if (wheel == 0.0f) {
			if (Input::GetKeyboardKey(DIK_W))wheel += 20.0f;
			if (Input::GetKeyboardKey(DIK_S))wheel -= 20.0f;
		}
		if (wheel)radius -= (wheel*circumference*0.1f);
		bool movePos = false; bool moveAt = false;
		if (upMove.LengthSq() == 0) {
			if (Input::GetKeyboardKey(DIK_Z)) {
				upMove += up * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_UP)) {
				upMove += up * 10.0f * circumference;
				moveAt = true;
			}
			if (Input::GetKeyboardKey(DIK_X)) {
				upMove -= up * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_DOWN)) {
				upMove -= up * 10.0f * circumference;
				moveAt = true;
			}
		}
		if (rightMove.LengthSq() == 0) {
			if (Input::GetKeyboardKey(DIK_D)) {
				rightMove += right * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_RIGHT)) {
				rightMove += right * 10.0f * circumference;
				moveAt = true;
			}
			if (Input::GetKeyboardKey(DIK_A)) {
				rightMove -= right * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_LEFT)) {
				rightMove -= right * 10.0f * circumference;
				moveAt = true;
			}
		}
		if (Input::GetMouseKey(0) || movePos) pos += rightMove + upMove;
		if (Input::GetMouseKey(1) || moveAt) at += rightMove + upMove;
		if (Input::GetMouseKey(2)) {
			pos += rightMove + upMove;
			at += rightMove + upMove;
		}
		if (radius < 2.0f)radius = 2.0f;
		front = TakeFront();
		pos = at - front * radius;
		right = Math::Vector3::Cross(up, front);
		up = Math::Vector3::Cross(front, right);
		//ここ外から追加できるようにしても良いが、外から使う予定もないためこれで
		//カメラをAOが映える場所へ
		if (Input::GetKeyboardKey(DIK_P)) {
			pos = Math::Vector3(57.0f, 66.0f, 106.0f);
			at = Math::Vector3();
			up = Math::Vector3(0.0f, 1.0f, 0.0f);
			radius = (pos - at).Length();
		}
		//ライトが映える場所へ
		if (Input::GetKeyboardKey(DIK_O)) {
			pos = Math::Vector3(-343.0f, 33.0f, -11.0f);
			at = Math::Vector3();
			up = Math::Vector3(0.0f, 1.0f, 0.0f);
			radius = (pos - at).Length();
		}
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
	void SceneDeferred::Initialize() {
		const constexpr Math::Vector2 scale(1280, 720);
		camera = std::make_unique<DebugCamera>(scale, Math::Vector3(57.0f, 66.0f, 106.0f), Math::Vector3(0.0f, 0.0f, 0.0f));
		deferredBuffer = std::make_unique<DeferredBuffer>(scale);
		normalMap = TRUE; useLight = TRUE; useFog = TRUE;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "normal map", &normalMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use fog", &useFog, false);
#endif
		//model = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		model = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
#ifdef SIMPLE_SHADER
		deferredShader = std::make_unique<SimpleDeferred>();
#endif
#ifdef FULL_EFFECT
		//光源設置
		deferredShader = std::make_unique<PointLightDeferred>();
		PointLightDeferred::PointLight light;
		for (int i = 0; i < LIGHT_COUNT; i++) {
			light.pos = Math::Vector4(Utility::Frand(-60.0f, 60.0f), Utility::Frand(-10.0f, 10.0f), Utility::Frand(-40.0f, 40.0f), 0.0f);
			light.pos += Math::Vector4(-213.0f, 5.0f, -5.0f, 0.0f);
			light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
			light.attenuation = Utility::Frand(0.5f, 10.0f);
			deferredShader->SetLightBuffer(i + 1, light);
		}
#endif
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao = std::make_unique<SSAOPS>(scale);
#endif
#ifdef SSAO_CS
		ssao = std::make_unique<SSAOCS>(scale*(QUALITY > 1.0f ? 1.0f : QUALITY));
#endif
#endif
#ifdef CASCADE
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 4, true);
#else
		shadow = std::make_unique<ShadowBuffer>(scale, 1, true);
#endif
		skybox = std::make_unique<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		//shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 1, false);
		//gaussian = std::make_unique<GaussianFilterCS>(scale);
	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		if (Input::GetKeyboardKey(DIK_7) == 1) normalMap = !normalMap;
		if (Input::GetKeyboardKey(DIK_6) == 1) useFog = !useFog;
		if (Input::GetKeyboardKey(DIK_5) == 1) useLight = !useLight;

		deferredBuffer->AddModel(model, normalMap);
#ifdef FULL_EFFECT
		//自分のカメラ位置にも光源を置く
		PointLightDeferred::PointLight light;
		//light.pos = Math::Vector4(pos.x, pos.y, pos.z, 0.0f);
		light.pos = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		light.color = 0xFFFFFFFF;
		//light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
		light.attenuation = 20.0f;
		deferredShader->SetLightBuffer(0, light);
		if (useLight)deferredShader->SetUseCount(LIGHT_COUNT + 1);
		else deferredShader->SetUseCount(0);
		deferredShader->Update();
#endif
		shadow->AddModel(model);
		shadow->SetPos(Math::Vector3(200.0f, 130.0f, 200.0f));
		//shadow->SetPos(pos);
		camera->Update();
	}
	void SceneDeferred::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->SetActiveLinearFog(useFog);
		Graphics::Environment::GetInstance()->SetFogBegin(150.0f);
		Graphics::Environment::GetInstance()->SetFogEnd(400.0f);
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		Graphics::RenderTarget* backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
		//shadow->CreateShadowMap(view.get(), backBuffer);
		shadow->CreateShadowMap(camera->GetView().get(), backBuffer);
		skybox->Render(camera.get());
		//view->Activate();
		shadow->Begin();
		deferredBuffer->RenderGBuffer();
		shadow->End();
		backBuffer->Activate();
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao->CreateAO(backBuffer, deferredBuffer.get());
#endif
#ifdef SSAO_CS
		ssao->CreateAO(backBuffer, camera->GetView().get(), deferredBuffer.get());
#endif
		ssao->Begin(5);
#endif
		deferredBuffer->Begin();
		deferredShader->Render();
		deferredBuffer->End();
#ifdef USE_SSAO
		ssao->End();
#endif
#ifdef _DEBUG
		if (Application::GetInstance()->debugRender) {
			deferredBuffer->DebugRender();
			shadow->DebugRender();
#ifdef USE_SSAO
			ssao->Render();
#endif
		}
#endif
	}
}