#pragma once
#pragma warning (disable:4100)
#include "Effekseer.h"
#include "EffekseerRendererDX11.h"
//#include "EffekseerSoundXAudio2.h"
#ifdef _DEBUG
#pragma comment(lib,"Effekseer.Debug.lib")
#pragma comment(lib,"EffekseerRendererDX11.Debug.lib")
//#pragma comment(lib,"EffekseerSoundXAudio2.Debug.lib")
#else
#pragma comment(lib,"Effekseer.Release.lib")
#pragma comment(lib,"EffekseerRendererDX11.Release.lib")
//#pragma comment(lib,"EffekseerSoundXAudio2.Release.lib")
#endif // _DEBUG
#pragma warning (default:4100)

namespace Lobelia::Graphics {
	inline namespace Experimental {
		using EffekseerData = ::Effekseer::Effect*;
		using EffekseerHandle = ::Effekseer::Handle;
		class EffekseerWrapper {
		private:
			static ::EffekseerRendererDX11::Renderer* renderer;
			/**@brief マネージャー*/
			static ::Effekseer::Manager* manager;
			/**@brief サウンドデバイス*/
			//static ::EffekseerSound::Sound* sound;
			/**@brief スプライトの最大数*/
			static const int SPRITE_MAX = 10000;
		private:
			static void Activate();
		public:
			static void Setting();
			static void Release();
			static EffekseerData Load(const char* file_path);
			static void Release(EffekseerData data);
			//static void SetListner(const Math::Vector3& pos, const Math::Vector3& front, const Math::Vector3& up_direct);
			static void SetPaused(EffekseerHandle handle, bool pause);
			static void SetPos(EffekseerHandle handle, const Math::Vector3& pos);
			static void SetScale(EffekseerHandle handle, const Math::Vector3& scale);
			static void SetScale(EffekseerHandle handle, float scale);
			static void SetRotation(EffekseerHandle handle, const Math::Vector3& rad);
			static void SetRotation(EffekseerHandle handle, const Math::Vector3& axis, float rad);
			static void SetSpeed(EffekseerHandle handle, float speed);
			static EffekseerHandle Play(EffekseerData data, const Math::Vector3& pos);
			static void Stop();
			static void Stop(EffekseerHandle handle);
			static bool IsExist(EffekseerHandle handle);
			static void Update(float time_scale = 1.0f);
			static void Render();
		};
	}
}
