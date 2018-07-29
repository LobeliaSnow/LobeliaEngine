#include "Lobelia.hpp"

namespace Lobelia::Graphics {
	namespace Experimental {
		::EffekseerRendererDX11::Renderer* EffekseerWrapper::renderer = nullptr;
		::Effekseer::Manager* EffekseerWrapper::manager = nullptr;
		//::EffekseerSound::Sound* EffekseerWrapper::sound = nullptr;

		void EffekseerWrapper::Activate() {
			::Effekseer::Matrix44 projection = {};
			::Effekseer::Matrix44 camera = {};
			DirectX::XMMATRIX proj = View::GetNowRowProjectionMatrix();
			DirectX::XMMATRIX view = View::GetNowRowViewMatrix();
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