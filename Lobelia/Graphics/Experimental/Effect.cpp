#include "Lobelia.hpp"

namespace Lobelia::Graphics {
	namespace Experimental {
		GaussianFilter::Data GaussianFilter::data;
		std::unique_ptr<ConstantBuffer<GaussianFilter::Data >> GaussianFilter::constantBuffer;
		std::unique_ptr<RenderTarget> GaussianFilter::renderTarget;
		std::unique_ptr<View> GaussianFilter::view;

		void GaussianFilter::Setting() {
			constantBuffer = std::make_unique<ConstantBuffer<Data>>(5, ShaderStageList::VS | ShaderStageList::PS);
			//ガウス用シェーダーコンパイル
			ShaderBank::Register<PixelShader>("GaussianFilterXPS", "Data/ShaderFile/2D/User.hlsl", "GaussianFilterXPS", PixelShader::Model::PS_4_0);
			ShaderBank::Register<PixelShader>("GaussianFilterYPS", "Data/ShaderFile/2D/User.hlsl", "GaussianFilterYPS", PixelShader::Model::PS_4_0);
			ShaderBank::Register<VertexShader>("GaussianFilterXVS", "Data/ShaderFile/2D/User.hlsl", "GaussianFilterXVS", VertexShader::Model::VS_4_0);
			ShaderBank::Register<VertexShader>("GaussianFilterYVS", "Data/ShaderFile/2D/User.hlsl", "GaussianFilterYVS", VertexShader::Model::VS_4_0);
		}
		void GaussianFilter::CreateBuffer(const Math::Vector2& scale) {
			data.size = scale;
			view = std::make_unique<View>(Math::Vector2(), scale);
			renderTarget = std::make_unique<RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 });
		}
		void GaussianFilter::BlurRender(View* now_view, RenderTarget* rt, Texture* tex) {
			constantBuffer->Activate(data);
			renderTarget->Clear(0x00000000);
			//X軸ブラーで別のバッファへ描画
			renderTarget->Activate();
			view->Activate();
			Pipeline* pipeline = PipelineManager::PipelineGet(D_PIPE2D_S);
			//X軸ブラー
			pipeline->VertexShaderSet("GaussianFilterXVS", 0, nullptr);
			pipeline->PixelShaderSet("GaussianFilterXPS", 0, nullptr);
			SpriteRenderer::Render(tex);
			//実レンダー用描画対象を有効化
			rt->Activate();
			//Y軸ブラー
			pipeline->VertexShaderSet("GaussianFilterYVS", 0, nullptr);
			pipeline->PixelShaderSet("GaussianFilterYPS", 0, nullptr);
			SpriteRenderer::Render(renderTarget->GetTexture());
			//パイプラインを初期に戻す
			pipeline->VertexShaderSet(D_VS2D, 0, nullptr);
			InstanceID id = D_PS2D_INS_TEX_ID;
			pipeline->PixelShaderSet(D_PS2D, 1, &id);
			now_view->Activate();
		}

		std::unique_ptr<RenderTarget> BlumePhongRenderer::diffuse;
		std::unique_ptr<RenderTarget> BlumePhongRenderer::specular;
		std::unique_ptr<RenderTarget> BlumePhongRenderer::gaussian;
		InstanceID BlumePhongRenderer::useNormalMap;
		InstanceID BlumePhongRenderer::noUseNormalMap;

		void BlumePhongRenderer::PreRenderWakeup() {
			RenderTarget* rts[2] = { diffuse.get(),specular.get() };
			RenderTarget::Activate(2, rts);
		}
		char* BlumePhongRenderer::GetPrePSName() {
			static char PS_NAME[16] = "PreBlumePhongPS";
			return PS_NAME;
		}
		BlumePhongRenderer::BlumePhongRenderer() {}
		BlumePhongRenderer::~BlumePhongRenderer() {}
		void BlumePhongRenderer::Setting(const Math::Vector2& back_buffer_size) {
			diffuse = std::make_unique<RenderTarget>(back_buffer_size, DXGI_SAMPLE_DESC{ 1,0 });
			//縮小バッファ作成
			specular = std::make_unique<RenderTarget>(back_buffer_size, DXGI_SAMPLE_DESC{ 1, 0 });
			//最後にぼかすのでバッファサイズが多少小さくても負荷軽減を優先
			gaussian = std::make_unique<RenderTarget>(back_buffer_size, DXGI_SAMPLE_DESC{ 1, 0 });
			//PS
			ShaderBank::Register<Graphics::PixelShader>(GetPrePSName(), "Data/Shaderfile/3D/PS.hlsl", GetPrePSName(), Graphics::PixelShader::Model::PS_5_0, true);
			PixelShader* ps = ShaderBank::Get<Graphics::PixelShader>(GetPrePSName());
			useNormalMap = ps->GetLinkage()->CreateInstance("UseNormalMapBlume");
			noUseNormalMap = ps->GetLinkage()->CreateInstance("NoUseNormalMapBlume");
			GaussianFilter::CreateBuffer(gaussian->GetTexture()->GetSize());
		}
		void BlumePhongRenderer::PreRender(Model* model, bool is_animation, bool is_normal_map) {
			PreRenderWakeup();
			Pipeline* p = nullptr;
			InstanceID use = noUseNormalMap;
			if (is_normal_map)use = useNormalMap;
			if (is_animation)p = PipelineManager::PipelineGet(D_PIPE3D_D);//スキニング用
			else p = PipelineManager::PipelineGet(D_PIPE3D_S);//スタティックメッシュ用
			p->PixelShaderSet(GetPrePSName(), 1, &use);
			model->Render();
			InstanceID id = D_PS3D_INS_PHONG_ID;
			p->PixelShaderSet(D_PS3D, 1, &id);
		}
		void BlumePhongRenderer::PreRender(ModelInstanced* model, bool is_normal_map) {
			PreRenderWakeup();
			Pipeline* p = PipelineManager::PipelineGet(D_PIPE3D_IS);
			InstanceID use = noUseNormalMap;
			if (is_normal_map)use = useNormalMap;
			p->PixelShaderSet(GetPrePSName(), 1, &use);
			model->Render();
			InstanceID id = D_PS3D_INS_PHONG_ID;
			p->PixelShaderSet(D_PS3D, 1, &id);
		}
		void BlumePhongRenderer::PreRender(ModelInstancedAnimation* model, bool is_normal_map) {
			PreRenderWakeup();
			Pipeline* p = PipelineManager::PipelineGet(D_PIPE3D_ID);
			InstanceID use = noUseNormalMap;
			if (is_normal_map)use = useNormalMap;
			p->PixelShaderSet(GetPrePSName(), 1, &use);
			model->Render();
			InstanceID id = D_PS3D_INS_PHONG_ID;
			p->PixelShaderSet(D_PS3D, 1, &id);
		}
		void BlumePhongRenderer::GaussianPhase(View* now_view) {
			GaussianFilter::BlurRender(now_view, gaussian.get(), specular->GetTexture());
		}
		void BlumePhongRenderer::PostRender() {
			Lobelia::Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
			Pipeline* p = Graphics::PipelineManager::PipelineGet(D_PIPE2D_S);
			p->BlendSet("Copy");
			SpriteRenderer::Render(diffuse->GetTexture());
			p->BlendSet("Add");
			SpriteRenderer::Render(gaussian->GetTexture());
			p->BlendSet("Copy");
		}
		void BlumePhongRenderer::ClearBuffer() {
			auto BufferClear = [=](RenderTarget* rt) {
				rt->Clear(0x00000000);
			};
			Texture::Clean(0, ShaderStageList::PS);
			Texture::Clean(1, ShaderStageList::PS);
			BufferClear(diffuse.get());
			BufferClear(specular.get());
			BufferClear(gaussian.get());
		}

		::EffekseerRendererDX11::Renderer* EffekseerWrapper::renderer = nullptr;
		::Effekseer::Manager* EffekseerWrapper::manager = nullptr;
		//::EffekseerSound::Sound* EffekseerWrapper::sound = nullptr;

		void EffekseerWrapper::Activate() {
			::Effekseer::Matrix44 projection = {};
			::Effekseer::Matrix44 camera = {};
			DirectX::XMMATRIX proj = View::GetNowRawProjectionMatrix();
			DirectX::XMMATRIX view = View::GetNowRawViewMatrix();
			memcpy_s(&projection, sizeof(::Effekseer::Matrix44), &proj, sizeof(DirectX::XMMATRIX));
			memcpy_s(&camera, sizeof(::Effekseer::Matrix44), &view, sizeof(DirectX::XMMATRIX));
			renderer->SetProjectionMatrix(projection);
			renderer->SetCameraMatrix(camera);
		}
		void EffekseerWrapper::Setting() {
			if (renderer)renderer->Destroy();
			renderer = ::EffekseerRendererDX11::Renderer::Create(Device::Get().Get(), Device::GetContext().Get(), SPRITE_MAX);
			if (!renderer)STRICT_THROW("エフェクシアレンダラの作成に失敗");
			if (manager)manager->Destroy();
			manager = ::Effekseer::Manager::Create(SPRITE_MAX);
			if (!manager)STRICT_THROW("エフェクシアマネージャーの作成に失敗");
			//sound = ::EffekseerSound::Sound::Create(Audio::Device::Get(), 1, 8);
			//if (!sound)STRICT_THROW("エフェクトサウンドインスタンスの作成");
			manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
			manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
			manager->SetRingRenderer(renderer->CreateRingRenderer());
			manager->SetTextureLoader(renderer->CreateTextureLoader());
			//manager->SetSoundPlayer(sound->CreateSoundPlayer());
			//manager->SetSoundLoader(sound->CreateSoundLoader());
			manager->SetCoordinateSystem(::Effekseer::CoordinateSystem::LH);
			manager->SetTrackRenderer(renderer->CreateTrackRenderer());
		}
		namespace {
			template<class T>inline void SafeDestroy(T* p) {
				p->Destroy();
				p = nullptr;
			}
		}
		void EffekseerWrapper::Release() {
			SafeDestroy(renderer);
			//renderer->Destory();
			SafeDestroy(manager);
			//SafeDestroy(sound);
		}
		EffekseerData EffekseerWrapper::Load(const char* file_path) {
			return ::Effekseer::Effect::Create(manager, reinterpret_cast<const EFK_CHAR*>(Utility::ConverteWString(file_path).c_str()));
		}
		void EffekseerWrapper::Release(EffekseerData data) { data->Release(); data = nullptr; }
		//void EffekseerWrapper::SetListner(const Math::Vector3& pos, const Math::Vector3& target, const Math::Vector3& up_direct) {
		//	Effekseer::Vector3D efkPos, efkTarget, efkUpDirect;
		//	memcpy_s(&efkPos, sizeof(Effekseer::Vector3D), &pos, sizeof(Math::Vector3));
		//	memcpy_s(&efkTarget, sizeof(Effekseer::Vector3D), &target, sizeof(Math::Vector3));
		//	memcpy_s(&efkUpDirect, sizeof(Effekseer::Vector3D), &up_direct, sizeof(Math::Vector3));
		//	sound->SetListener(efkPos, efkTarget, efkUpDirect);
		//}
		void EffekseerWrapper::SetPaused(EffekseerHandle handle, bool pause) {
			manager->SetPaused(handle, pause);
		}
		void EffekseerWrapper::SetPos(EffekseerHandle handle, const Math::Vector3& pos) {
			Effekseer::Vector3D efkPos = {};
			memcpy_s(&efkPos, sizeof(Effekseer::Vector3D), &pos, sizeof(Math::Vector3));
			manager->SetLocation(handle, efkPos);
		}
		void EffekseerWrapper::SetScale(EffekseerHandle handle, const Math::Vector3& scale) {
			manager->SetScale(handle, scale.x, scale.y, scale.z);
		}
		void EffekseerWrapper::SetScale(EffekseerHandle handle, float scale) {
			manager->SetScale(handle, scale, scale, scale);
		}
		void EffekseerWrapper::SetRotation(EffekseerHandle handle, const Math::Vector3& rad) {
			manager->SetRotation(handle, rad.x, rad.y, rad.z);
		}
		void EffekseerWrapper::SetRotation(EffekseerHandle handle, const Math::Vector3& axis, float rad) {
			Effekseer::Vector3D efkAxis = {};
			memcpy_s(&efkAxis, sizeof(Effekseer::Vector3D), &axis, sizeof(Math::Vector3));
			manager->SetRotation(handle, efkAxis, rad);
		}
		void EffekseerWrapper::SetSpeed(EffekseerHandle handle, float speed) {
			manager->SetSpeed(handle, speed);
		}
		EffekseerHandle EffekseerWrapper::Play(EffekseerData data, const Math::Vector3& pos) {
			return manager->Play(data, pos.x, pos.y, pos.z);
		}
		void EffekseerWrapper::Stop() {
			manager->StopAllEffects();
		}
		void EffekseerWrapper::Stop(EffekseerHandle handle) {
			manager->StopEffect(handle);
		}
		bool EffekseerWrapper::IsExist(EffekseerHandle handle) {
			return manager->Exists(handle);
		}
		void EffekseerWrapper::Update(float time_scale) {
			manager->BeginUpdate();
			manager->Update(time_scale);
			manager->EndUpdate();
		}
		void EffekseerWrapper::Render() {
			Activate();
			//renderer->EndRendering()でのシェーダーリンケージエラー回避
			Device::GetContext()->PSSetShader(nullptr, nullptr, 0);
			renderer->BeginRendering();
			manager->Draw();
			renderer->EndRendering();
		}
	}
}