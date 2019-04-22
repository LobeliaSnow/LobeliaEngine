#include "Lobelia.hpp"
#include "SceneMain.hpp"

#include "Wininet.h"
#include "Shlobj.h"

//個人製作 第二弾
//第三弾はグラフィック的に良いものを作ろうと画策中

namespace Lobelia::Game {
	//研究スペース
	//namespace Develop {
	//	Material::Material(const char* file_path, const char* instance_name) {

	//	}

	//	void Renderer::SetMaterial(std::shared_ptr<Material>& material) { this->material = material; }
	//	std::shared_ptr<Material>& Renderer::GetMaterial() { return material; }
	//	void Renderer::SetVertexShader(std::shared_ptr<Graphics::VertexShader>& vs) { this->vs = vs; }
	//	std::shared_ptr<Graphics::VertexShader>&  Renderer::GetVertexShader() { return vs; }
	//	void Renderer::SetPixelShader(std::shared_ptr<Graphics::PixelShader>& ps) { this->ps = ps; }
	//	std::shared_ptr<Graphics::PixelShader>&  Renderer::GetPixelShader() { return ps; }

	//	std::shared_ptr<Graphics::VertexShader> ShaderBufferObject::CreateVertexShader(const char* include, const char* vs) {

	//	}
	//	std::shared_ptr<Graphics::PixelShader> ShaderBufferObject::CreatePixelShader(const char* include, const char* ps) {

	//	}

	//	MeshRenderer::MeshRenderer() {

	//	}
	//	void MeshRenderer::SetFilter(std::shared_ptr<MeshFilter> mesh) {

	//	}
	//	void MeshRenderer::Render() {

	//	}

	//	SkinMeshRenderer::SkinMeshRenderer() {

	//	}
	//	void SkinMeshRenderer::Render() {

	//	}
	//}
	std::shared_ptr<Audio::Buffer> buffer;
	std::shared_ptr<Audio::SourceVoice> voice;

	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
		//SetWindowLong(Application::GetInstance()->GetWindow()->GetHandle(), GWL_STYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
		//if (SetLayeredWindowAttributes(Application::GetInstance()->GetWindow()->GetHandle(), RGB(0, 0, 0), 0, LWA_COLORKEY | LWA_ALPHA) == 0)
		//{
		//	STRICT_THROW("ウインドウの設定変更に失敗");
		//}

		HRESULT _hr = S_OK;
		Microsoft::WRL::ComPtr<IActiveDesktop> activeDesktop;
		_hr = CoCreateInstance(CLSID_ActiveDesktop, 0, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, r_cast<void**>(activeDesktop.GetAddressOf()));
		if (FAILED(_hr))
		{
			STRICT_THROW("インスタンス作成に失敗");
		}

		//_hr = activeDesktop->SetWallpaper(L"C:/Users/black.MSI/Desktop/LobeliaEngine/test.jpg", 0);
		////_hr = activeDesktop->SetWallpaper(L"test.jpg", 0);
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("壁紙変更設定に失敗");
		//}
		//_hr = activeDesktop->ApplyChanges(AD_APPLY_ALL);
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("変更の適用に失敗");
		//}
		static wchar_t a[1024000000000000000] = {};
		_hr = activeDesktop->GetWallpaper(a, 1024000000000000000, AD_GETWP_BMP);
		if (FAILED(_hr))
		{
			STRICT_THROW("背景を取得できませんでした");
		}
		//COMPONENT component;
		//_hr = activeDesktop->GenerateDesktopItemHtml(L"C:/Users/black.MSI/Desktop/LobeliaEngine/test.html", &component, 0);
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("HTMLの保存に失敗");
		//}

		//_factory->MakeWindowAssociation
		//_factory->GetWindowAssociation

		//Lobelia::Graphics::DisplayInfoList::Bootup();
		//int count = Lobelia::Graphics::DisplayInfoList::GetDisplayCount();
		//auto display = Lobelia::Graphics::DisplayInfoList::GetDisplayInfo(0);
		//output = r_cast<IDXGIOutput1*>(display->output.Get());
		//HRESULT _hr = S_OK;
		//_hr = output->DuplicateOutput(Lobelia::Graphics::Device::Get().Get(), outputDuplication.GetAddressOf());
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("DuplicateOutputの取得に失敗");
		//}
		//SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, "C:/Users/black.MSI/Desktop/LobeliaEngine/test.bmp", SPIF_SENDWININICHANGE);

#if 0
		//ホワイトノイズ
		buffer = std::make_shared<Audio::Buffer>();
		buffer->format.wFormatTag = WAVE_FORMAT_PCM;
		buffer->format.nChannels = 1;
		buffer->format.wBitsPerSample = 8;
		buffer->format.nSamplesPerSec = 44100;
		buffer->format.nBlockAlign = buffer->format.wBitsPerSample / 8 * buffer->format.nChannels;
		buffer->format.nAvgBytesPerSec = buffer->format.nSamplesPerSec * buffer->format.nBlockAlign;
		buffer->size = 441000;
		buffer->source = std::make_unique<BYTE[]>(buffer->size);
		float hz = Audio::Device::CalcMIDIHz(69);
		for (int i = 0; i < buffer->size / sizeof(short); i++) {
			buffer->source[i] = rand();
		}
		voice = std::make_shared<Audio::SourceVoice>(buffer);
		voice->Play(0);
#endif
#if 0
		//周波数から音を鳴らす
		buffer = std::make_shared<Audio::Buffer>();
		buffer->format.wFormatTag = WAVE_FORMAT_PCM;
		buffer->format.nChannels = 1;
		buffer->format.wBitsPerSample = 16;
		buffer->format.nSamplesPerSec = 44100;
		buffer->format.nBlockAlign = buffer->format.wBitsPerSample / 8 * buffer->format.nChannels;
		buffer->format.nAvgBytesPerSec = buffer->format.nSamplesPerSec * buffer->format.nBlockAlign;
		buffer->size = 441000;
		buffer->source = std::make_unique<BYTE[]>(buffer->size);
		short* temp = r_cast<short*>(buffer->source.get());
		//ラ
		float hz = Audio::Device::CalcMIDIHz(69);
		float length = buffer->format.nSamplesPerSec / hz;
		for (int i = 0; i < buffer->size / sizeof(short); i++) {
			temp[i] = s_cast<short>(32767.0f * sinf(f_cast(i) * PI / (length / 2)));
		}
		voice = std::make_shared<Audio::SourceVoice>(buffer);
		voice->Play(0);
#endif
#if 0
		//周波数から音を鳴らす
		buffer = std::make_shared<Audio::Buffer>();
		buffer->format.wFormatTag = WAVE_FORMAT_PCM;
		buffer->format.nChannels = 1;
		buffer->format.wBitsPerSample = 16;
		buffer->format.nSamplesPerSec = 44100;
		buffer->format.nBlockAlign = buffer->format.wBitsPerSample / 8 * buffer->format.nChannels;
		buffer->format.nAvgBytesPerSec = buffer->format.nSamplesPerSec * buffer->format.nBlockAlign;
		buffer->size = 441000;
		buffer->source = std::make_unique<BYTE[]>(buffer->size);
		short* temp = r_cast<short*>(buffer->source.get());
		//ラ
		float hz = Audio::Device::CalcMIDIHz(69);
		float length = buffer->format.nSamplesPerSec / hz;
		for (int i = 0; i < buffer->size / sizeof(short); i++) {
			temp[i] = s_cast<short>(32767.0f * sinf(f_cast(i) * PI / (length / 2)));
		}
		voice = std::make_shared<Audio::SourceVoice>(buffer);
		voice->Play(0);
#endif
#if 0
		Audio::Bank::GetInstance()->Load("06.EGOIST - 名前のない怪物（wav）.wav", "BGM");
		Audio::Player::GetInstance()->Play("BGM", 0);
#endif
	}
	SceneMain::~SceneMain() {
		voice.reset();
	}
	void SceneMain::Initialize() {
	}
	void SceneMain::AlwaysUpdate() {
	}
	void SceneMain::AlwaysRender() {
		view->Activate();
		HRESULT _hr = S_OK;
#if 0
		//output->TakeOwnership
		//output->GetFrameStatistics
		//DXGI_MAPPED_RECT _rect = {};
		//_hr = outputDuplication->MapDesktopSurface(&_rect);
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("デスクトップイメージの取得に失敗");
		//}
#endif
#if 0
		//ComPtr<ID3D11Texture2D> _texture;
		//ComPtr<IDXGIResource> _desktopResource;
		//DXGI_OUTDUPL_FRAME_INFO _frameInfo;
		////Get new frame
		//_hr = outputDuplication->AcquireNextFrame(500, &_frameInfo, _desktopResource.GetAddressOf());
		//if (FAILED(_hr))
		//{
		//	if ((_hr != DXGI_ERROR_ACCESS_LOST) && (_hr != DXGI_ERROR_WAIT_TIMEOUT))
		//	{
		//		STRICT_THROW("Failed to acquire next frame in DUPLICATIONMANAGER");
		//	}
		//}
		//HANDLE sharedHandle;
		//_hr = _desktopResource->GetSharedHandle(&sharedHandle);
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("共有ハンドルの取得に失敗");
		//}
		//_hr = Graphics::Device::Get()->OpenSharedResource(sharedHandle, IID_PPV_ARGS(_texture.GetAddressOf()));
		//if (FAILED(_hr))
		//{
		//	STRICT_THROW("共有ハンドルを開けませんでした");
		//}
		////_hr = _desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture.GetAddressOf()));
		////if (FAILED(_hr))
		////{
		////	STRICT_THROW("テクスチャの取得に失敗");
		////}
		//d3d11Texture = std::make_shared<Graphics::RenderTarget>(_texture);
		////d3d11Texture->Activate();
		////Graphics::SpriteRenderer::Render(s_cast<Graphics::Texture*>(nullptr));
		//Graphics::SpriteRenderer::Render(d3d11Texture.get());
		////Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		//outputDuplication->ReleaseFrame();
#endif
	}
}
