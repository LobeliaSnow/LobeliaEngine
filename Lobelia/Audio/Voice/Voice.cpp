#include "Common/Common.hpp"
#include "Audio/Device/Device.hpp"
#include "Audio/Voice/Voice.hpp"
#include "Audio/Bank/Bank.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Audio {
	namespace _ {
		/**@brief voice解放用*/
		void SafeDestroyVoice(IXAudio2Voice* voice) {
			if (voice) {
				voice->DestroyVoice();
				voice = nullptr;
			}
		}
	}
	IXAudio2MasteringVoice* MasterVoice::master = nullptr;
	void MasterVoice::Create() {
		HRESULT hr = Device::Get()->CreateMasteringVoice(&master);
		if (FAILED(hr))STRICT_THROW("マスタリングボイスの作成に失敗");
	}
	void MasterVoice::Destroy() { _::SafeDestroyVoice(master); }

	namespace {
		XAUDIO2FX_REVERB_I3DL2_PARAMETERS PRESET_PARAMS[30] = {
			XAUDIO2FX_I3DL2_PRESET_FOREST,
			XAUDIO2FX_I3DL2_PRESET_DEFAULT,
			XAUDIO2FX_I3DL2_PRESET_GENERIC,
			XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
			XAUDIO2FX_I3DL2_PRESET_ROOM,
			XAUDIO2FX_I3DL2_PRESET_BATHROOM,
			XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
			XAUDIO2FX_I3DL2_PRESET_STONEROOM,
			XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
			XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
			XAUDIO2FX_I3DL2_PRESET_CAVE,
			XAUDIO2FX_I3DL2_PRESET_ARENA,
			XAUDIO2FX_I3DL2_PRESET_HANGAR,
			XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
			XAUDIO2FX_I3DL2_PRESET_HALLWAY,
			XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
			XAUDIO2FX_I3DL2_PRESET_ALLEY,
			XAUDIO2FX_I3DL2_PRESET_CITY,
			XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
			XAUDIO2FX_I3DL2_PRESET_QUARRY,
			XAUDIO2FX_I3DL2_PRESET_PLAIN,
			XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
			XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
			XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
			XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
			XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
			XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
			XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
			XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
			XAUDIO2FX_I3DL2_PRESET_PLATE,
		};
	}
	IUnknown* EffectVoice::reverb = nullptr;
	IXAudio2SubmixVoice* EffectVoice::submixVoice = nullptr;

	void EffectVoice::Create() {
		DWORD flags = 0;
#ifdef _DEBUG
		flags |= XAUDIO2FX_DEBUG;
#endif
		HRESULT hr = S_OK;
		//リバーブ作成
		hr = XAudio2CreateReverb(&reverb, flags);
		if (FAILED(hr))STRICT_THROW("リバーブの作成に失敗しました");
		//エフェクトチェイン
		XAUDIO2_EFFECT_DESCRIPTOR descriptor = {};
		descriptor.InitialState = true;
		descriptor.OutputChannels = 6;
		descriptor.pEffect = reverb;
		XAUDIO2_EFFECT_CHAIN chain = {};
		chain.EffectCount = 1;
		chain.pEffectDescriptors = &descriptor;
		hr = Device::Get()->CreateSubmixVoice(&submixVoice, 2, 44100, 0, 0, nullptr, &chain);
		if (FAILED(hr))STRICT_THROW("サブミックスボイスの作成に失敗しました");
	}
	void EffectVoice::Destroy() {
		_::SafeDestroyVoice(submixVoice);
		reverb->Release();
		reverb = nullptr;
	}
	IXAudio2SubmixVoice* EffectVoice::GetSubmixVoice() { return submixVoice; }
	void EffectVoice::SetEffect(EFFECT_ELEMENT element, int index) {
		//プリセットのエフェクトを適用
		XAUDIO2FX_REVERB_PARAMETERS native;
		ReverbConvertI3DL2ToNative(&PRESET_PARAMS[i_cast(element)], &native);
		submixVoice->SetEffectParameters(index, &native, sizeof(native));
	}
	void EffectVoice::EnableEffect(int index) {
		submixVoice->EnableEffect(index);
	}
	void EffectVoice::DisableEffect(int index) {
		submixVoice->DisableEffect(index);
	}
	X3DAUDIO_HANDLE Sound3DSystem::instanceHandle = {};
	X3DAUDIO_LISTENER Sound3DSystem::listner = {};
	namespace {
		const X3DAUDIO_CONE LISTENER_DIRECTIONAL_CONE = { X3DAUDIO_PI * 5.0f / 6.0f, X3DAUDIO_PI * 11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };
	}
	void Sound3DSystem::SetUpDirection(const Math::Vector3& up) {
		memcpy_s(&listner.OrientTop, sizeof(listner.OrientTop), &up, sizeof(up));
	}
	void Sound3DSystem::CalculationUpDirection(const Math::Vector3& pos, const Math::Vector3& front) {
		Math::Vector3 up(0.0f, 1.0f, 0.0f);
		//右方向算出
		Math::Vector3 right = Math::Vector3::Cross(up, front);
		//上方向算出
		up = Math::Vector3::Cross(front, right);
		SetUpDirection(up);
	}

	void Sound3DSystem::Setting(const Math::Vector3& pos, const Math::Vector3& front) {
		HRESULT hr = X3DAudioInitialize(Device::GetDetails().OutputFormat.dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, instanceHandle);
		if (FAILED(hr))STRICT_THROW("3D音源システム初期化に失敗");
		//リスナーの初期情報セット
		listner.pCone = const_cast<X3DAUDIO_CONE*>(&LISTENER_DIRECTIONAL_CONE);
		SetListnerPos(pos);
		SetListnerFrontVector(front);
		CalculationUpDirection(pos, front);
	}
	const X3DAUDIO_HANDLE& Sound3DSystem::GetHandle() { return instanceHandle; }
	const X3DAUDIO_LISTENER& Sound3DSystem::GetListner() { return listner; }
	void Sound3DSystem::SetListnerPos(const Math::Vector3& pos) {
		memcpy_s(&listner.Position, sizeof(listner.Position), &pos, sizeof(pos));
	}
	void Sound3DSystem::SetListnerFrontVector(const Math::Vector3& front) {
		memcpy_s(&listner.OrientFront, sizeof(listner.OrientFront), &front, sizeof(front));
	}
	Emitter::Emitter(const Math::Vector3& pos, const Math::Vector3& front) :pos(pos), front(front), emitter{}, dsp{} {
		//あってるかな？
		emitter.ChannelCount = 2;
		emitter.CurveDistanceScaler = FLT_MIN;
		dsp.SrcChannelCount = 1;
		dsp.DstChannelCount = 6;
		dsp.pMatrixCoefficients = matrix;
		emitter.ChannelRadius = 10.0f;
		SetSpeed(this->front);
		SetImpactDistanceScaler(10.0f);
		SetDopplerScaler(1.0f);
		SetPos(this->pos);
		SetFrontVector(this->front);
		this->pos.Normalize();
		this->front.Normalize();
		//上方向算出&セット
		CalculationUpDirection(this->pos, this->front);
		//コーンの設定
		emitter.pCone = &cone;
		emitter.pCone->InnerAngle = X3DAUDIO_2PI;
		emitter.pCone->InnerVolume = 2.0f;
		emitter.pCone->OuterVolume = 1.0f;
		emitter.pCone->InnerLPF = 0.0f;
		emitter.pCone->OuterLPF = 0.0f;
		emitter.pCone->InnerReverb = 1.0f;
		emitter.pCone->OuterReverb = 1.0f;

		emitter.ChannelRadius = 1.0f;
		emitter.pChannelAzimuths = azimuths;
		emitter.InnerRadius = 2.0f;
		emitter.InnerRadiusAngle = X3DAUDIO_PI / 4.0;
		emitter.pVolumeCurve = (X3DAUDIO_DISTANCE_CURVE*)&X3DAudioDefault_LinearCurve;
		emitter.pLPFDirectCurve = nullptr; // use default curve
		emitter.pLPFReverbCurve = nullptr; // use default curve
		//emitter.pCone->OuterAngle = 1.0f;
	}
	Emitter::~Emitter() = default;
	void Emitter::SetUpDirection(const Math::Vector3& up) {
		memcpy_s(&emitter.OrientTop, sizeof(emitter.OrientTop), &up, sizeof(up));
	}
	void Emitter::CalculationUpDirection(const Math::Vector3& pos, const Math::Vector3& front) {
		Math::Vector3 up(0.0f, 1.0f, 0.0f);
		//右方向算出
		Math::Vector3 right = Math::Vector3::Cross(up, front);
		//上方向算出
		up = Math::Vector3::Cross(front, right);
		SetUpDirection(up);
	}
	void Emitter::SetPos(const Math::Vector3& pos) {
		this->pos = pos;
		memcpy_s(&emitter.Position, sizeof(emitter.Position), &pos, sizeof(pos));
	}
	void Emitter::SetFrontVector(const Math::Vector3& front) {
		this->front = front;
		memcpy_s(&emitter.OrientFront, sizeof(emitter.OrientFront), &front, sizeof(front));
	}
	void Emitter::SetDopplerScaler(float scaler) { emitter.DopplerScaler = scaler; }
	void Emitter::SetImpactDistanceScaler(float scaler) { emitter.CurveDistanceScaler = scaler; }
	void Emitter::SetSpeed(const Math::Vector3& speed) {
		memcpy_s(&emitter.Velocity, sizeof(emitter.Velocity), &speed, sizeof(speed));
	}
	void Emitter::Update(SourceVoice* voice) {
		//上方向算出&セット
		CalculationUpDirection(pos, front);
		//3D音源アップデート
		X3DAudioCalculate(Sound3DSystem::GetHandle(), &Sound3DSystem::GetListner(), &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB | X3DAUDIO_CALCULATE_REVERB, &dsp);
		std::shared_ptr<Buffer> buffer = voice->buffer.lock();
		if (!buffer)STRICT_THROW("期間切れのボイスです");
		voice->voice->SetOutputMatrix(EffectVoice::GetSubmixVoice(), buffer->format.nChannels, 2, dsp.pMatrixCoefficients);
		voice->voice->SetFrequencyRatio(dsp.DopplerFactor);
		XAUDIO2_FILTER_PARAMETERS FilterParametersDirect = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp.LPFDirectCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
		voice->voice->SetOutputFilterParameters(EffectVoice::GetSubmixVoice(), &FilterParametersDirect);
		XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp.LPFReverbCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
		voice->voice->SetOutputFilterParameters(EffectVoice::GetSubmixVoice(), &FilterParametersReverb);
	}
	//Callbackは後で実装
	SourceVoice::SourceVoice(std::weak_ptr<Buffer> buffer) :buffer(buffer) {
		std::shared_ptr<Buffer> instance = buffer.lock();
		if (!instance)STRICT_THROW("音声データが存在しません");
		XAUDIO2_SEND_DESCRIPTOR sendDescriptors;
		sendDescriptors.Flags = XAUDIO2_SEND_USEFILTER; // LPF direct-path
		sendDescriptors.pOutputVoice = EffectVoice::GetSubmixVoice();
		const XAUDIO2_VOICE_SENDS sendList = { 1, &sendDescriptors };
		HRESULT	hr = Device::Get()->CreateSourceVoice(&voice, &instance->format, 0 | XAUDIO2_VOICE_USEFILTER  /*| XAUDIO2_VOICE_MUSIC*/, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &sendList, nullptr);
		if (FAILED(hr))STRICT_THROW("ソースボイスが作成できませんでした");
	}
	SourceVoice::~SourceVoice() { _::SafeDestroyVoice(voice); }
	void SourceVoice::Play(UINT loop) {
		std::shared_ptr<Buffer> instance = buffer.lock();
		if (!instance)STRICT_THROW("音声データが存在しません");
		data.Flags = XAUDIO2_END_OF_STREAM;
		data.pAudioData = instance->source.get();
		data.AudioBytes = instance->size;
		data.PlayBegin = 0;
		data.PlayLength = instance->size / instance->format.nBlockAlign;
		data.LoopBegin = 0;
		data.LoopCount = loop >= XAUDIO2_LOOP_INFINITE ? XAUDIO2_LOOP_INFINITE : loop;
		data.LoopLength = loop ? instance->size / instance->format.nBlockAlign : 0;
		voice->SubmitSourceBuffer(&data, nullptr);
		voice->Start(0, XAUDIO2_COMMIT_NOW);
		isPlay = true;
	}
	void SourceVoice::Pause()const noexcept {
		voice->Stop(0, XAUDIO2_COMMIT_NOW);
	}
	void SourceVoice::Stop()noexcept {
		voice->Stop(XAUDIO2_PLAY_TAILS, XAUDIO2_COMMIT_ALL);
		voice->FlushSourceBuffers();
		//キューに追加
		voice->SubmitSourceBuffer(&data, nullptr);
		isPlay = false;
	}

	bool SourceVoice::IsPlay()const noexcept {
		if (!isPlay)return false;
		XAUDIO2_VOICE_STATE state = {};
		voice->GetState(&state);
		return (state.BuffersQueued != 0);
	}
	void SourceVoice::SetVolume(float volume) { voice->SetVolume(volume); }

	void Player::Update() {
		for (auto&& voice = voices.begin(); voice != voices.end();) {
			if (!voice->second->IsPlay()) {
				voice->second->Stop();
				if (voice->first.is3D)emitters.erase(voice->first);
				voice = voices.erase(voice);
			}
			else {
				if (voice->first.is3D)emitters[voice->first]->Update(voice->second.get());
				voice++;
			}
		}
	}
	const Player::Handle Player::IssueHandle(bool is_3d) {
		static UINT next = 0;
		Handle handle(next++, is_3d);
		return handle;
	}
	const Player::Handle Player::Play(const char* tag, int loop, bool is_3d) {
		Handle handle = IssueHandle(is_3d);
		voices[handle] = (std::make_unique<SourceVoice>(Bank::GetInstance()->GetBuffer(tag)));
		if (is_3d) emitters[handle] = std::make_shared<Emitter>(Math::Vector3(0.0f, 0.0f, 0.0f), Math::Vector3(0.0f, 0.0f, 1.0f));
		voices[handle]->Play(loop);
		return handle;
	}
	void Player::Stop(const Handle& handle) {
		if (!FindSoundHandle(handle))STRICT_THROW("存在しないハンドルです");
		voices[handle]->Stop();
	}
	void Player::Pause(const Handle& handle) {
		if (!FindSoundHandle(handle))STRICT_THROW("存在しないハンドルです");
		voices[handle]->Pause();
	}
	bool Player::IsPlay(const Handle& handle) {
		if (!FindSoundHandle(handle))STRICT_THROW("存在しないハンドルです");
		return voices[handle]->IsPlay();
	}
	void Player::SetVolume(const Handle& handle, float volume) {
		if (!FindSoundHandle(handle))STRICT_THROW("存在しないハンドルです");
		voices[handle]->SetVolume(volume);
	}
	std::weak_ptr<Emitter> Player::GetEmitter(Handle handle) {
		if (!handle.is3D || FindEmitterHundle(handle))STRICT_THROW("3Dとして再生されていないか、存在しないハンドルです");
		return emitters[handle];
	}
	void Player::Clear() {
		for each(auto&& voice in voices) {
			voice.second->Stop();
		}
		voices.clear();
		emitters.clear();
	}
	size_t Player::TakeSize() { return voices.size(); }
	bool Player::FindSoundHandle(const Handle& handle) { return (voices.find(handle) != voices.end()); }
	bool Player::FindEmitterHundle(const Handle& handle) { return (emitters.find(handle) != emitters.end()); }
}