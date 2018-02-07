#pragma once
namespace Lobelia::Audio {
	class MasterVoice {
	private:
		static IXAudio2MasteringVoice* master;
	public:
		static void Create();
		static void Destroy();
	};
	class EffectVoice {
	public:
		enum class EFFECT_ELEMENT { FOREST, DEFAULT, GENERIC, PADDEDCELL, ROOM, BATHROOM, LIVINGROOM, STONEROOM, AUDITORIUM, CONCERTHALL, CAVE, ARENA, HANGAR, CARPETEDHALLWAY, HALLWAY, STONECORRIDOR, ALLEY, CITY, MOUNTAINS, QUARRY, PLAIN, PARKINGLOT, SEWERPIPE, UNDERWATER, SMALLROOM, MEDIUMROOM, LARGEROOM, MEDIUMHALL, LARGEHALL, PLATE };
	private:
		static IUnknown* reverb;
		static IXAudio2SubmixVoice* submixVoice;
	public:
		static void Create();
		static void Destroy();
		static IXAudio2SubmixVoice* GetSubmixVoice();
		static void SetEffect(EFFECT_ELEMENT element, int index);
		static void EnableEffect(int index);
		static void DisableEffect(int index);
	};
	class Sound3DSystem {
	private:
		static X3DAUDIO_HANDLE instanceHandle;
		static X3DAUDIO_LISTENER listner;
	private:
		static void SetUpDirection(const Math::Vector3& up);
		static void CalculationUpDirection(const Math::Vector3& pos, const Math::Vector3& front);
	public:
		static void Setting(const Math::Vector3& pos, const Math::Vector3& front);
		static const X3DAUDIO_HANDLE& GetHandle();
		static const X3DAUDIO_LISTENER& GetListner();
		static void SetListnerPos(const Math::Vector3& pos);
		static void SetListnerFrontVector(const Math::Vector3& front);
	};
	class Emitter {
	private:
		X3DAUDIO_EMITTER emitter;
		X3DAUDIO_DSP_SETTINGS dsp;
		float matrix[12];
		X3DAUDIO_CONE cone;
		FLOAT32 azimuths[2];
		//計算用
		Math::Vector3 pos;
		Math::Vector3 front;
	private:
		void SetUpDirection(const Math::Vector3& up);
		void CalculationUpDirection(const Math::Vector3& pos, const Math::Vector3& front);
	public:
		Emitter(const Math::Vector3& pos, const Math::Vector3& front);
		~Emitter();
		void SetPos(const Math::Vector3& pos);
		void SetFrontVector(const Math::Vector3& front);
		void SetDopplerScaler(float scaler);
		void SetImpactDistanceScaler(float scaler);
		void SetSpeed(const Math::Vector3& speed);
		void Update(class SourceVoice* voice);
	};
	struct Buffer {
		WAVEFORMATEX format = {};
		BYTE* source = nullptr;
		UINT size = s_cast<UINT>(-1);
	};
	class SourceVoice {
		friend class Emitter;
	private:
		IXAudio2SourceVoice * voice;
		XAUDIO2_BUFFER data;
		Buffer buffer;
	public:
		SourceVoice(const Buffer& buffer);
		~SourceVoice();
		//MAX 255
		void Play(UINT loop)noexcept;
		void Pause()const noexcept;
		void Stop()const noexcept;
		bool IsPlay()const noexcept;
		void SetVolume(float volume);
	};
	class Player {
	private:
		Buffer buffer;
		std::list<std::unique_ptr<SourceVoice>> voices;
	public:
		Player(const Buffer& buffer);
		~Player();
		void Update();
		void Play(UINT loop);
		void Stop();
		void Pause();
		bool IsPlay();
		void SetVolume(float volume);
		size_t TakeSize();
		//const std::list<std::unique_ptr<SourceVoice>>& GetVoices();
	};
	struct Voice3D {
		std::unique_ptr<SourceVoice> voice;
		std::unique_ptr<Emitter> emitter;
	};
	struct Voice3DHandle {
		DWORD handle;
		Voice3DHandle(std::nullptr_t null) { handle = -1; }
		Voice3DHandle() {
			static DWORD handleMax = 0;
			handle = handleMax++;
			if (handleMax >= UINT_MAX)handleMax = 0;
		}
	};
	inline bool operator <(const Voice3DHandle& data0, const Voice3DHandle& data1) { return (data0.handle < data1.handle); }
	//異なるプレイヤーのインスタンスでBufferを使いまわすことはできません
	class Voice3DPlayer {
	private:
		Buffer buffer;
		std::map<Voice3DHandle, Voice3D>voices;
	public:
		Voice3DPlayer(const Buffer& buffer);
		~Voice3DPlayer();
		Voice3DHandle Play(const Math::Vector3& pos, const Math::Vector3& front, bool loop);
		//falseが返ってきた場合、そのインスタンスは既に存在しない
		bool Stop(Voice3DHandle handle);
		bool Pause(Voice3DHandle handle);
		bool IsPlay(Voice3DHandle handle);
		bool SetVolume(Voice3DHandle handle, float volume);
		bool SetPos(Voice3DHandle handle, const Math::Vector3& pos);
		bool SetFrontVector(Voice3DHandle handle, const Math::Vector3& front);
		bool SetDopplerScaler(Voice3DHandle handle, float scaler);
		bool SetImpactDistanceScaler(Voice3DHandle handle, float scaler);
		bool SetSpeed(Voice3DHandle handle, const Math::Vector3& speed);
		void Update();
		size_t TakeSize();

	};
}