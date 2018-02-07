#pragma once
namespace Lobelia::Audio {
	class Bank {
	public:
		static void Load(const char* file_path, const char* tag);
		static void Load3D(const char* file_path, const char* tag);
		static void Play(const char* tag, UINT loop = 0);
		static Voice3DHandle Play3D(const char* tag, const Math::Vector3& pos, const Math::Vector3& front, bool loop = false);
		//現在このタグで再生中の音声をすべて止める
		static void Stop(const char* tag);
		static bool Stop3D(const char* tag, Voice3DHandle handle);
		//現在このタグで再生中の音声をすべて一時停止
		static void Pause(const char* tag);
		static bool Pause3D(const char* tag, Voice3DHandle handle);
		//現在このタグで再生中の音声が一つでもあるかどうか取得
		static bool IsPlay(const char* tag);
		static bool IsPlay3D(const char* tag, Voice3DHandle handle);
		static void SetVolume(const char* tag, float volume);
		static bool SetVolume3D(const char* tag, Voice3DHandle handle, float volume);
		static bool SetPos3D(const char* tag, Voice3DHandle handle, const Math::Vector3& pos);
		static bool SetFrontVector3D(const char* tag, Voice3DHandle handle, const Math::Vector3& front);
		static bool SetDopplerScaler3D(const char* tag, Voice3DHandle handle, float scaler);
		static bool SetImpactDistanceScaler3D(const char* tag, Voice3DHandle handle, float scaler);
		static bool SetSpeed3D(const char* tag, Voice3DHandle handle, const Math::Vector3& speed);
		static Player* GetData(const char* tag);
		static Voice3DPlayer* GetData3D(const char* tag);
		static void Clear();
		static void Clear3D();
		static void Clear(const char* tag);
		static void Clear3D(const char* tag);
		static void Update();
	};
}