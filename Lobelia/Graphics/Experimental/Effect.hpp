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
		//テクスチャをガウスぼかしで描画
		class GaussianFilter {
			ALIGN(16)struct Data {
				Math::Vector2 size;
			};
			static Data data;
			static std::unique_ptr<ConstantBuffer<Data >> constantBuffer;
			static std::unique_ptr<RenderTarget> renderTarget;
			static std::unique_ptr<View> view;
		public:
			static void Setting();
			static void CreateBuffer(const Math::Vector2& scale);
			static void BlurRender(View* now_view, RenderTarget* rt, Texture* tex);
		};

		//参考
		//http://maverickproj.web.fc2.com/pg32.html
		//blume phong
		//モデルの更新処理は各自呼んでください
		//使い方はPreRender->GaussianPhase->PostRender
		class BlumePhongRenderer {
		public:
			static std::unique_ptr<RenderTarget> diffuse;
			static std::unique_ptr<RenderTarget> specular;
			static std::unique_ptr<RenderTarget> gaussian;
			static InstanceID useNormalMap;
			static InstanceID noUseNormalMap;
		private:
			static void PreRenderWakeup();
			static char* GetPrePSName();
		public:
			BlumePhongRenderer();
			~BlumePhongRenderer();
			static void Setting(const Math::Vector2& back_buffer_size);
			static void PreRender(Model* model, bool is_animation, bool is_normal_map = false);
			static void PreRender(ModelInstanced* model, bool is_normal_map = false);
			static void PreRender(ModelInstancedAnimation* model, bool is_normal_map = false);
			static void GaussianPhase(View* now_view);
			static void PostRender();
			static void ClearBuffer();
		};

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
