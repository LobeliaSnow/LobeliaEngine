#include "Lobelia.hpp"
#include "SceneMain.hpp"

//TODO : 描画ステートのパイプラインクラスを調整
//TODO : コントローラー入力
//TODO : テクスチャブレンドや雨等
//TODO : トーン入出力処理(DXGI)を実装？
//TODO : deltabaseでの飛び問題解決
//Raypickする際に-1されていることを忘れてはならない。
//TODO : Sound 矩形波を流し込めるようにしたりするのを簡単にする

namespace Lobelia::Game {
	//midiのノート番号を引数で渡すと、その周波数が返ってきます
	float CalcMIDIHz(int note) {
		return 440.0f * f_cast(pow(2.0, (s_cast<double>(note) - 69.0) / 12.0f));
	}
	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
		Audio::Buffer buffer;
		buffer.format.nChannels = 1;
		buffer.format.nSamplesPerSec = 44100;
		buffer.format.wBitsPerSample = 16;
		buffer.format.nBlockAlign = s_cast<short>(buffer.format.wBitsPerSample / 8 * buffer.format.nChannels);
		buffer.format.nAvgBytesPerSec = buffer.format.nSamplesPerSec*buffer.format.nBlockAlign;
		buffer.format.wFormatTag = WAVE_FORMAT_PCM;
		buffer.source = new BYTE[sizeof(short)*buffer.format.nAvgBytesPerSec];
		buffer.size = sizeof(short)*buffer.format.nAvgBytesPerSec;
		short* temp = r_cast<short*>(buffer.source);
		//ラ
		float hz = CalcMIDIHz(69);
		for (int i = 0; i < buffer.size / sizeof(short); i++) {
			//音の波形のサイズは-32767.0~32767.0までなので、それを超えると波形がラリる可能性がある
			//https://drumimicopy.com/audio-frequency/
			temp[i] = s_cast<short>(32767.0f*sinf(2.0f*PI* hz *s_cast<float>(i) / s_cast<float>(buffer.format.nSamplesPerSec)));
		}
		Audio::EffectVoice::DisableEffect(0);
		Audio::SourceVoice voice(buffer);
		voice.Play(0);
		while (voice.IsPlay());
		voice.Stop();
		delete[] buffer.source;
	}
	SceneMain::~SceneMain() {
	}
	void SceneMain::Initialize() {	}
	void SceneMain::Update() {
	}
	void SceneMain::Render() {
		view->Activate();
	}
}
