#include "Lobelia.hpp"

namespace Lobelia {
	//protected
	bool Application::IsUpdate() {
		timer->End();
		processTimer = timer->GetMilisecondResult();
		if (processTimer <= 1000.0f / Config::GetRefPreference().applicationOption.updateFPS)return false;
		timer->Begin();

		return true;
	}

	void Application::Update() {
		Audio::Bank::Update();
		Input::DeviceManager::Update();
		Game::GameObject2DManager::GetInstance()->Update();
		SceneManager::GetInstance()->Update();
	}

	void Application::Render() {
		swapChain->Clear(0x00000000);
#ifdef USE_IMGUI_AND_CONSOLE
		ImGui_ImplDX11_NewFrame();
#endif
		SceneManager::GetInstance()->Render();
		Game::GameObject2DManager::GetInstance()->Render();
		if (Config::GetRefPreference().applicationOption.systemVisible)FpsRender();
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->UpdateProcess();
		HostConsole::GetInstance()->UpdateAndRender();
		ImGui::Render();
#endif
		swapChain->Present();
	}

	void Application::FpsRender() {
#ifdef USE_IMGUI_AND_CONSOLE
		const Math::Vector2& pos = Config::GetRefPreference().applicationOption.pos;
		const Math::Vector2& size = Config::GetRefPreference().applicationOption.size;
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);
		ImGui::Begin("System State", nullptr, ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		if (ImGui::CollapsingHeader("FPS", ImGuiTreeNodeFlags_DefaultOpen))	ImGui::Text("FPS -> %.3f", CalcFps());
		if (ImGui::CollapsingHeader("Process Time", ImGuiTreeNodeFlags_DefaultOpen))	ImGui::Text("Process Time -> %.3f", processTimer);
		ImGui::End();
#endif
	}
	Application::Application() : processTimer(0.0f), timer(std::make_unique<Timer>())/*, changeScene(false), tempScene(nullptr) */ {}
	Application::~Application() = default;
	float Application::CalcFps() { return 1000.0f / processTimer; }

	Window* Application::GetWindow() { return window.get(); }
	Graphics::SwapChain* Application::GetSwapChain() { return swapChain.get(); }
	void Application::ResizeBuffer() {
		swapChain;
	}
	WPARAM Application::Run() {
		MSG msg = {};
		while (msg.message != WM_QUIT) {
			//‘æ“ñˆø”‚ªNULLo‚È‚¢‚Æƒ‹[ƒv‚ð”²‚¯‚Ä‚­‚ê‚È‚¢
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
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->ProcessUnRegister("command clear");
		HostConsole::GetInstance()->CommandUnRegister("screen shot");
		HostConsole::GetInstance()->VariableUnRegister("analyze domain");
		ImGui_ImplDX11_Shutdown();
#endif
	}
	float Application::GetProcessTime() { return processTimer; }
}