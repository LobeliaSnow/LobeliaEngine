#include "Lobelia.hpp"

namespace Lobelia {
	bool Application::IsUpdate() {
		timer->End();
		processTimer = timer->GetMilisecondResult();
		if (processTimer <= 1000.0f / Config::GetRefPreference().updateFPS)return false;
		HostConsole::GetInstance()->SetProcessTime(processTimer);
		timer->Begin();

		return true;
	}

	void Application::Update() {
#ifdef _DEBUG
		Graphics::DebugRenderer::GetInstance()->Begin();
#endif
		Audio::Player::GetInstance()->Update();
		Input::Keyboard::GetInstance()->Update();
		Input::Mouse::GetInstance()->Update();
		Input::Joystick::GetInstance()->Update();
		//Game::GameObject2DManager::GetInstance()->Update();
		SceneManager::GetInstance()->Update();
	}

	void Application::Render() {
		swapChain->Clear(0x00000000);
#ifdef USE_IMGUI_AND_CONSOLE
		ImGui_ImplDX11_NewFrame();
#endif
		SceneManager::GetInstance()->Render();
#ifdef _DEBUG
		Graphics::DebugRenderer::GetInstance()->End();
#endif
		//Game::GameObject2DManager::GetInstance()->Render();
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->UpdateProcess();
		HostConsole::GetInstance()->UpdateAndRender();
		ImGui::Render();
#endif
		swapChain->Present();
	}

	Application::Application() : processTimer(0.0f), timer(std::make_unique<Timer>()), timeScale(1.0f)/*, changeScene(false), tempScene(nullptr) */ {}
	Application::~Application() = default;
	float Application::CalcFps() { return 1000.0f / processTimer; }

	Window* Application::GetWindow() { return window.get(); }
	Graphics::SwapChain* Application::GetSwapChain() { return swapChain.get(); }
	WPARAM Application::Run() {
		MSG msg = {};
		while (msg.message != WM_QUIT) {
			//第二引数がNULL出ないとループを抜けてくれない
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				if (!IsUpdate())continue;
				Update();
				Render();
			}
		}
		return msg.wParam;
	}

	void Application::Shutdown() {
		swapChain->Get()->SetFullscreenState(false, nullptr);
		SceneManager::GetInstance()->Clear();
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->ProcessUnRegister("command clear");
		HostConsole::GetInstance()->CommandUnRegister("screen shot");
		HostConsole::GetInstance()->VariableUnRegister("analyze domain");
		ImGui_ImplDX11_Shutdown();
#endif
	}
	float Application::GetProcessTimeMili() { return processTimer * timeScale; }
	float Application::GetProcessTimeSec() { return processTimer * 0.001f * timeScale; }
	void Application::SetTimeScale(float time_scale) { timeScale = time_scale; }
}