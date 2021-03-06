#include "Lobelia.hpp"
#include "SceneMain.hpp"

//TODO : 描画ステートのパイプラインクラスを調整
//TODO : コントローラー入力
//TODO : テクスチャブレンドや雨等
//TODO : deltabaseでの飛び問題解決
//Raypickする際に-1されていることを忘れてはならない。
//TODO : Sound 矩形波を流し込めるようにしたりするのを簡単にする

namespace Lobelia::Game {
	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
		std::shared_ptr<Audio::Buffer> buffer= std::make_shared<Audio::Buffer>();
		buffer->format.nChannels = 1;
		buffer->format.nSamplesPerSec = 44100;
		buffer->format.wBitsPerSample = 16;
		buffer->format.nBlockAlign = s_cast<short>(buffer->format.wBitsPerSample / 8 * buffer->format.nChannels);
		buffer->format.nAvgBytesPerSec = buffer->format.nSamplesPerSec*buffer->format.nBlockAlign;
		buffer->format.wFormatTag = WAVE_FORMAT_PCM;
		//秒数
		buffer->size = buffer->format.nAvgBytesPerSec * 1;
		buffer->source = std::make_unique<BYTE[]>(buffer->size);
		short* temp = r_cast<short*>(buffer->source.get());
		//ラ
		float hz = Audio::Device::CalcMIDIHz(69);
		for (int i = 0; i < buffer->size / sizeof(short); i++) {
			//音の波形のサイズは-32767.0~32767.0までなので、それを超えると波形がラリる可能性がある
			//https://drumimicopy.com/audio-frequency/
			temp[i] = s_cast<short>(32767.0f*sinf(2.0f*PI* hz *s_cast<float>(i) / s_cast<float>(buffer->format.nSamplesPerSec)));
		}
		Audio::EffectVoice::DisableEffect(0);
		Audio::SourceVoice voice(std::move(buffer));
		voice.Play(0);
		while (voice.IsPlay());
		voice.Stop();
	}
	SceneMain::~SceneMain() {
	}
	void SceneMain::Initialize() {
	}
	void SceneMain::AlwaysUpdate() {
	}
	void SceneMain::AlwaysRender() {
		view->Activate();
	}
}
