/**
*@mainpage Hdx2017 DirectX11使用
*@brief 未完成<br>
*@brief フレームワークからアプリケーションクラス(新規作成)に乗り換える
*/

/**
*@file WinMain.cpp
*@brief メイン
*@author Lobelia_Snow
*/

#include "Lobelia.hpp"
#include "SceneMain.hpp"

#ifdef _DEBUG	
//メモリリーク検知用
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG

//ウインドウプロシージャです
LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)noexcept {
	Lobelia::Input::DeviceManager::UpdateProc(hwnd, msg, wp, lp);
#ifdef USE_IMGUI_AND_CONSOLE
	ImGui_ImplDX11_WndProcHandler(hwnd, msg, wp, lp);
#endif
	switch (msg) {
	case WM_CLOSE:					DestroyWindow(hwnd);						break;
	case WM_DESTROY:				PostQuitMessage(0);							break;
	case WM_KEYDOWN:				if (wp == VK_ESCAPE) PostMessage(hwnd, WM_CLOSE, 0, 0);	 return 0;
	case WM_SYSKEYDOWN:		if (wp != VK_MENU) return (DefWindowProc(hwnd, msg, wp, lp)); break;
	case WM_MOVE:					break;
	case WM_SIZE:
		Lobelia::Application::GetInstance()->GetWindow()->DetectionSize();
		//Lobelia::Application::GetInstance()->ResizeBuffer();		
		break;
	default:		return (DefWindowProc(hwnd, msg, wp, lp));
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
#ifdef _DEBUG
	//メモリリーク検知用
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	//_crtBreakAlloc = 521;
#endif	//_DEBUG
	WPARAM wp = {};
	try {
		Lobelia::Bootup();
		Lobelia::Application::GetInstance()->Bootup<Lobelia::Game::SceneMain>(Lobelia::Math::Vector2(1280, 720), ENGINE_VERSION, WndProc);
		Lobelia::Audio::EffectVoice::DisableEffect(0);

#ifdef USE_IMGUI_AND_CONSOLE
#if defined( _DEBUG)
		Lobelia::HostConsole::GetInstance()->ProcessRegister("ss", [=]() {
			static char filePath[256] = { "SS.png" };
			if (Lobelia::Input::DeviceManager::GetKey<Lobelia::Input::Keyboard>(VK_F3) == 1) {
				Lobelia::Graphics::TextureFileAccessor::Save(filePath, Lobelia::Application::GetInstance()->GetSwapChain()->GetRenderTarget()->GetTexture());
			}
		});
		//Lobelia::Application::GetInstance()->Bootup<Application::SceneRanking>(Lobelia::Math::Vector2(1280, 720), ENGINE_VERSION, WndProc, Lobelia::RankingData<float>("Data/Score/data.dat", 10, 999.99), Lobelia::Utility::Frand(0.0f, 0.5f));
#else
		Lobelia::Config::GetRefPreference().consoleOption.active = false;
		Lobelia::Config::GetRefPreference().applicationOption.systemVisible = false;
#endif
#endif
		Lobelia::Graphics::BlumePhongRenderer::Setting(Lobelia::Application::GetInstance()->GetWindow()->GetSize());
		wp = Lobelia::Application::GetInstance()->Run();
		Lobelia::Application::GetInstance()->Shutdown();
		Lobelia::Graphics::EffekseerWrapper::Release();
		Lobelia::Shutdown();
	}
	catch (const Lobelia::Exception& exception) {
		exception.BoxMessage();
		return -1;
	}
	return static_cast<int>(wp);
}
