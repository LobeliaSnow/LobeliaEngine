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
	class SourceVoice;
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
		void Update(SourceVoice* voice);
	};
	//音バッファ
	struct Buffer {
		WAVEFORMATEX format = {};
		std::unique_ptr<BYTE[]> source;
		int size;
	};
	class SourceVoice {
		friend class Emitter;
	private:
		IXAudio2SourceVoice * voice;
		XAUDIO2_BUFFER data;
		std::weak_ptr<Buffer> buffer;
		bool isPlay;
	public:
		SourceVoice(std::weak_ptr<Buffer> buffer);
		~SourceVoice();
		//MAX 255
		void Play(UINT loop);
		void Pause()const noexcept;
		void Stop()noexcept;
		bool IsPlay()const noexcept;
		void SetVolume(float volume);
	};
	class Player :public Utility::Singleton<Player> {
		friend class Utility::Singleton<Player>;
	public:
		//音声ハンドル
		struct Handle {
			friend class Player;
			friend bool operator<(const Handle& h0, const Handle& h1);
		public:
			//無効なハンドルの生成
			Handle() :id(UINT_MAX), is3D(false) {}
		private:
			UINT id;
			bool is3D;
		private:
			Handle(UINT id, bool is_3d) :id(id), is3D(is_3d) {}
		};
	public:
		//サウンドの管理
		void Update();
		//再生関連
		const Handle Play(const char* tag, int loop, bool is_3d = false);
		void Stop(const Handle& handle);
		void Pause(const Handle& handle);
		bool IsPlay(const Handle& handle);
		void SetVolume(const Handle& handle, float volume);
		//3D関連
		std::weak_ptr<Emitter> GetEmitter(Handle handle);
		//その他
		void Clear();
		size_t TakeSize();
		bool FindSoundHandle(const Handle& handle);
		bool FindEmitterHundle(const Handle& handle);
	private:
		//ハンドルを発行する
		const Handle IssueHandle(bool is_3d);
	private:
		Player() = default;
		~Player() = default;
	public:
		Player(const Player&) = delete;
		Player(Player&&) = delete;
		Player& operator=(const Player&) = delete;
		Player& operator=(Player&&) = delete;
	private:
		std::map<const Handle, std::unique_ptr<SourceVoice>> voices;
		std::map<const Handle, std::shared_ptr<Emitter>> emitters;
	};
	inline bool operator<(const Player::Handle& h0, const Player::Handle& h1) { return (h0.id < h1.id); }
}