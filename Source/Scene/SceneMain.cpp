#include "Lobelia.hpp"
#include "SceneMain.hpp"

//TODO : �`��X�e�[�g�̃p�C�v���C���N���X�𒲐�
//TODO : �R���g���[���[����
//TODO : �e�N�X�`���u�����h��J��
//TODO : �g�[�����o�͏���(DXGI)�������H
//TODO : deltabase�ł̔�і�����
//Raypick����ۂ�-1����Ă��邱�Ƃ�Y��Ă͂Ȃ�Ȃ��B
//TODO : Sound ��`�g�𗬂����߂�悤�ɂ����肷��̂��ȒP�ɂ���

namespace Lobelia::Game {
	//midi�̃m�[�g�ԍ��������œn���ƁA���̎��g�����Ԃ��Ă��܂�
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
		//��
		float hz = CalcMIDIHz(75);
		int loopCount = buffer.size / sizeof(short);
		for (int i = 0; i < loopCount; i++) {
			if (i == loopCount / 2)hz = CalcMIDIHz(69);
			//���̔g�`�̃T�C�Y��-32767.0~32767.0�܂łȂ̂ŁA����𒴂���Ɣg�`��������\��������
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
