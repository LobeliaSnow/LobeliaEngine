#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//ComputeShader��p����SSAO������Ă݂���
//ComputeShader��p�����A���`�G�C���A�X������Ă݂���

namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 127;
	}
	DeferredBuffer::DeferredBuffer(const Math::Vector2& size) :size(size) {
		for (int i = 0; i < 4; i++) {
			rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 });
		}
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
		for (int i = 0; i < 4; i++) {
			rts[i]->Clear(0x00000000);
		}
		Graphics::RenderTarget::Activate(rts[0].get(), rts[1].get(), rts[2].get(), rts[3].get());
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
		for (int i = 0; i < 4; i++) {
			rts[i]->GetTexture()->Set(i, Graphics::ShaderStageList::PS);
		}
	}
	void DeferredBuffer::End() {
		for (int i = 0; i < 4; i++) {
			Graphics::Texture::Clean(i, Graphics::ShaderStageList::PS);
		}
	}
	void DeferredBuffer::DebugRender() {
		for (int i = 0; i < 4; i++) {
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
	PointLightDeferred::PointLightDeferred() : DeferredShader("Data/ShaderFile/3D/deferred.hlsl", "", "PointLightDeferredPS") {
		cbuffer = std::make_unique<Graphics::ConstantBuffer<PointLights>>(6, Graphics::ShaderStageList::PS);
		sizeof(PointLights);
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

	ShadowBuffer::ShadowBuffer(const Math::Vector2& size, int split_count) {
		rts.resize(split_count);
		for (int i = 0; i < split_count; i++) {
			rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 });
		}
	}
	PostEffect::PostEffect(const Math::Vector2& size, bool create_rt) :size(size) {
		if (create_rt)rt = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 });
	}
	std::shared_ptr<Graphics::RenderTarget>& PostEffect::GetRenderTarget() {
		if (!rt)STRICT_THROW("�쐬����Ă��Ȃ��o�b�t�@�ł�");
		return rt;
	}
	void PostEffect::Render() { Graphics::SpriteRenderer::Render(rt.get()); }
	void PostEffect::Begin(int slot) { this->slot = slot; }
	//��{�I�Ƀ|�X�g�G�t�F�N�g�̓s�N�Z���V�F�[�_�[�ł����g���Ȃ�
	void PostEffect::End() { Graphics::Texture::Clean(slot, Graphics::ShaderStageList::PS); }
	UnorderedAccessView::UnorderedAccessView(Graphics::Texture* texture) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(texture->Get().Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAV�̍쐬�Ɏ��s");
	}
	void UnorderedAccessView::Set(int slot) {
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, uav.GetAddressOf(), nullptr);
	}
	void UnorderedAccessView::Clean(int slot) {
		ID3D11UnorderedAccessView* null = nullptr;
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, &null, nullptr);
	}
	SSAO::SSAO(const Math::Vector2& size) :PostEffect(size, false) {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "SSAOCS");
		rwTexture = std::make_shared<Graphics::Texture>(size, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		//rt = std::make_shared<Graphics::RenderTarget>(rwTexture);
		uav = std::make_unique<UnorderedAccessView>(rwTexture.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(7, Graphics::ShaderStageList::CS | Graphics::ShaderStageList::PS);
		info.offsetPerPixel = 20.0f;
		info.useAO = TRUE;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use AO", &info.useAO, false);
		HostConsole::GetInstance()->FloatRegister("deferred", "use offset per pixel", &info.offsetPerPixel, false);
#endif
	}
	void SSAO::CreateAO(DeferredBuffer* deferred_buffer) {
		//int slot, ID3D11ShaderResourceView* uav
		if (Input::GetKeyboardKey(DIK_SPACE) == 1)info.useAO = !info.useAO;
		cbuffer->Activate(info);
		deferred_buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS)->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		uav->Set(0);
		const Math::Vector2& size = Application::GetInstance()->GetWindow()->GetSize();
		cs->Run(((int)size.x + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, ((int)size.y + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, 1);
		uav->Clean(0);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::CS);
	}
	void SSAO::Render() {
		Graphics::SpriteRenderer::Render(rwTexture.get(), Math::Vector2(4 * 200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), rwTexture->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void SSAO::Begin(int slot) {
		PostEffect::Begin(slot);
		rwTexture->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	GaussianFilter::GaussianFilter(const Math::Vector2& size) :PostEffect(size, true) {
		csX = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterCSX");
		csY = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterCSY");
		rwTexturePass1 = std::make_shared<Graphics::Texture>(size, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		rwTexturePass2 = std::make_shared<Graphics::Texture>(size, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		uavPass1 = std::make_unique<UnorderedAccessView>(rwTexturePass1.get());
		uavPass2 = std::make_unique<UnorderedAccessView>(rwTexturePass2.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(8, Graphics::ShaderStageList::CS);
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		dispersion = 0.03f;
	}
	void GaussianFilter::SetDispersion(float dispersion) { this->dispersion = dispersion; }
	void GaussianFilter::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture) {
		if (size != texture->GetSize()) {
			//�𑜓x���킹��A��{�_�E���T���v�����O�����
			this->rt->Clear(0x00000000);
			this->rt->Activate();
			view->ViewportActivate();
			Graphics::SpriteRenderer::Render(texture);
			active_rt->Activate();
			active_view->ViewportActivate();
			this->rt->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		}
		else rt->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		static constexpr const UINT DIVISION = 7;
		if (dispersion <= 0.0f)dispersion = 0.01f;
		float total = 0.0f;
		// �K�E�X�֐��ɂ��d�݂̌v�Z
		for (int i = 0; i < DIVISION; i++) {
			float pos = (float)i * 2.0f;
			info.weight[i] = expf(-pos * pos * dispersion);
			total += info.weight[i];
		}
		// �d�݂̋K�i��
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
	void GaussianFilter::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt) { Dispatch(active_view, active_rt, rt->GetTexture()); }
	void GaussianFilter::Render() {
		Graphics::SpriteRenderer::Render(rwTexturePass2.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void GaussianFilter::Begin(int slot) {
		PostEffect::Begin(slot);
		rwTexturePass2->Set(this->slot, Graphics::ShaderStageList::PS);
	}

	void SceneDeferred::Initialize() {
		const constexpr Math::Vector2 scale(1280, 720);
		view = std::make_unique<Graphics::View>(Math::Vector2(), scale);
		deferredBuffer = std::make_unique<DeferredBuffer>(scale);
		normalMap = TRUE; useLight = TRUE;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "normal map", &normalMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
#endif
		pos = Math::Vector3(57.0f, 66.0f, 106.0f);
		at = Math::Vector3(0.0f, 0.0f, 0.0f);
		up = Math::Vector3(0.0f, 1.0f, 0.0f);
		//model = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		model = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
#ifdef SIMPLE_SHADER
		deferredShader = std::make_unique<SimpleDeferred>();
#endif
#ifdef POINT_LIGHT
		//�����ݒu
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
		ssao = std::make_unique<SSAO>(scale);
#endif
		shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 4);
		gaussian = std::make_unique<GaussianFilter>(scale*0.5f);
	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		//�J�����ړ�
		//���Z�o
		Math::Vector3 front;
		Math::Vector3 right;
		front = pos - at; front.Normalize();
		right = Math::Vector3::Cross(up, front);
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		//�O��
		if (Input::GetKeyboardKey(DIK_S)) pos += front * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_W)) pos -= front * 20.0f*elapsedTime;
		//���E%
		if (Input::GetKeyboardKey(DIK_A)) pos += right * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_D)) pos -= right * 20.0f*elapsedTime;
		//�㉺
		if (Input::GetKeyboardKey(DIK_Z)) pos += up * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_X)) pos -= up * 20.0f*elapsedTime;
		//�J������AO���f����ꏊ��
		if (Input::GetKeyboardKey(DIK_P))pos = Math::Vector3(57.0f, 66.0f, 106.0f);
		//���C�g���f����ꏊ��
		if (Input::GetKeyboardKey(DIK_O))pos = Math::Vector3(-343.0f, 33.0f, -11.0f);
		//+1�̓J�����ʒu�̃J�����̕�
		deferredShader->SetUseCount(LIGHT_COUNT + 1);
		//�J�����X�V
		view->SetEyePos(pos);
		view->SetEyeTarget(at);
		view->SetEyeUpDirection(up);
		deferredBuffer->AddModel(model, normalMap);
#ifdef POINT_LIGHT
		//�����̃J�����ʒu�ɂ�������u��
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
	}
	void SceneDeferred::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->Activate();
		view->Activate();
		deferredBuffer->RenderGBuffer();
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
#ifdef USE_SSAO
		ssao->CreateAO(deferredBuffer.get());
#endif
		ssao->Begin(4);
		deferredBuffer->Begin();
		deferredShader->Render();
		deferredBuffer->End();
		ssao->End();
		//gaussian->Dispatch(view.get(), Application::GetInstance()->GetSwapChain()->GetRenderTarget(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::COLOR));
		//gaussian->Render();
#ifdef _DEBUG
		if (Application::GetInstance()->debugRender) {
			deferredBuffer->DebugRender();
#ifdef USE_SSAO
			ssao->Render();
#endif
		}
#endif
	}
}