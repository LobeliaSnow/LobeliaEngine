#include "Lobelia.hpp"

//template RenderableObject�݂����Ȃ̂�����Ă���ɃX�e�[�g���V�F�[�_�[����ۑ�����̔h���ŕ`�敔�������
//2DSprite�̓����_���`���ɕύX
//TODO : pipeline�V�X�e���͍��₷��

namespace Lobelia {
	void Bootup() {
//#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
//		//https://github.com/Microsoft/DirectXTex/wiki/DirectXTex
//		Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
//		if (FAILED(initialize))STRICT_THROW("COM�̏������Ɏ��s���܂���");
//#else
//#endif
		if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))STRICT_THROW("COM�̏������Ɏ��s���܂���");
		srand(s_cast<unsigned>(time(NULL)));
		XMLSystem::Initialize();
		Config::LoadSetting("Data/Config/Config.xml");
		Input::DirectInput::GetInstance()->Initialize();

		//�O���t�B�b�N�h���C�o����
		/*Graphics::GraphicDriverInfoList::Bootup();
		int driverCount = Graphics::GraphicDriverInfoList::GetGraphicDriverCount();
		Graphics::GraphicDriverInfo* driver = nullptr;
		for (int i = 0; i < driverCount; i++) {
			Graphics::GraphicDriverInfo* info = Graphics::GraphicDriverInfoList::GetDriver(i).get();
			if (!driver)driver = info;
			else if (info->GetVideoMemory() > driver->GetVideoMemory())driver = info;
		}*/
		//�f�o�C�X�쐬
		DWORD flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		Graphics::Device::Create(flag, /*driver*/nullptr);
		//Graphics::GraphicDriverInfoList::Shutdown();
		Graphics::Direct2DSystem::Initialize();
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->Bootup();
#endif
		Audio::Device::Create();
		Audio::MasterVoice::Create();
		Audio::EffectVoice::Create();
		Audio::Sound3DSystem::Setting({}, { 0.0f,0.0f,1.0f });
		Audio::Bank::GetInstance()->AttachLoader(&Audio::Loader::Wav, ".wav");
		Audio::Bank::GetInstance()->AttachLoader(&Audio::Loader::Ogg, ".ogg");
		Network::System::Startup();
		Network::SocketList::Initialize();

		Graphics::SpriteRenderer::Initialize();
		
		Lobelia::Graphics::Environment::GetInstance()->SetAmbientColor(0xFFFFFFFF);
		Lobelia::Math::Vector3 lightDir(1.0f, -1.0f, 1.0f); lightDir.Normalize();
		Lobelia::Graphics::Environment::GetInstance()->SetLightDirection(lightDir);
		Lobelia::Graphics::Environment::GetInstance()->Activate();

		Lobelia::Graphics::EffekseerWrapper::Setting();

		//Dijkstra�̃N�G���[�����炩���ߐݒ肵�Ă���
		Lobelia::Game::DijkstraEngineVector3::SetQueryCostFunction([](const Math::Vector3& p0, const Math::Vector3& p1) {return (p0 - p1).LengthSq(); });
		Lobelia::Game::DijkstraEngineVector3::SetQueryFunction([](const Math::Vector3& p0, const Math::Vector3& p1) {return (p0 - p1).LengthSq(); });
		Lobelia::Game::DijkstraEngineVector2::SetQueryCostFunction([](const Math::Vector2& p0, const Math::Vector2& p1) {return (p0 - p1).LengthSq(); });
		Lobelia::Game::DijkstraEngineVector2::SetQueryFunction([](const Math::Vector2& p0, const Math::Vector2& p1) {return (p0 - p1).LengthSq(); });
	}

	void Shutdown() {
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->Shutdown();
#endif
		Audio::Player::GetInstance()->Clear();
		Audio::EffectVoice::Destroy();
		Audio::MasterVoice::Destroy();
		Audio::Device::Destroy();
		Network::System::Cleanup();
		//Movie::MovieSystem::Shutdown();
		Network::System::Cleanup();
		CoUninitialize();
	}
}
