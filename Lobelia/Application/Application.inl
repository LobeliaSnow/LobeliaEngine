#pragma once
namespace Lobelia {
	template<class Scene, class ...Args> void Application::Bootup(const Math::Vector2& size, const char* window_name, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)>wnd_proc, Args&&... args) {
#ifdef _DEBUG
		debugRender = true;
		HostConsole::GetInstance()->ProcessRegister("debug render on off", [this]() {if (Input::GetKeyboardKey(DIK_F6) == 1) debugRender = !debugRender; });
		Graphics::DebugRenderer::GetInstance()->Initialize();
#endif
		window = std::make_shared<Window>(size, window_name, wnd_proc, WS_OVERLAPPEDWINDOW);
		Input::Keyboard::GetInstance()->Initialize(window->GetHandle());
		Input::Mouse::GetInstance()->Initialize(window->GetHandle());
		Input::Joystick::GetInstance()->Initialize(window->GetHandle());
		//æ“¾o—ˆ‚½Joystick‚Ì—ñ‹“
		for (int i = 0; i < Input::Joystick::GetInstance()->GetControllerCount(); i++) {
			std::string name = Input::Joystick::GetInstance()->GetDeviceName(i);
			HostConsole::GetInstance()->SetLog(name);
		}
		HostConsole::GetInstance()->Printf("joystick count : %d", Input::Joystick::GetInstance()->GetControllerCount());
		swapChain = std::make_unique<Graphics::SwapChain>(window.get(), Config::GetRefPreference().msaa);
		SceneManager::GetInstance()->ChangeReserve<Scene>(std::forward<Args>(args)...);
		window->ShowWindow(SW_SHOW);
		window->UppdateWindow();
		timer->Begin();
		Graphics::Direct2DRenderer::Initialize();
#ifdef USE_IMGUI_AND_CONSOLE
		ImGui_ImplDX11_Init(window->GetHandle(), Graphics::Device::Get().Get(), Graphics::Device::GetContext().Get());
		HostConsole::GetInstance()->ProcessRegister("command clear", [=]() {if (Input::GetKeyboardKey(DIK_F2) == 1)HostConsole::GetInstance()->ClearCommand(); });
		HostConsole::GetInstance()->ProcessRegister("console change visible", [=]() {
			if (Input::GetKeyboardKey(DIK_F1) == 1) {
				Config::GetRefPreference().consoleOption.active = !Config::GetRefPreference().consoleOption.active;
				Config::GetRefPreference().consoleOption.systemVisible = !Config::GetRefPreference().consoleOption.systemVisible;
			}
		});
		HostConsole::GetInstance()->CommandRegister("screen shot", HostConsole::ExeStyle::ALWAYS, [=]() {
			static char filePath[256] = {};
			ImGui::InputText("path", filePath, 256);
			if (ImGui::Button("command execute")) {
				Graphics::TextureFileAccessor::Save(filePath, swapChain->GetRenderTarget()->GetTexture());
				strcpy_s(filePath, "");
				return true;
			}
			return false;
		});
		HostConsole::GetInstance()->FloatRegister("analyze domain", "domain", &Config::GetRefPreference().consoleOption.variableAnalyzeDomain, false);
#endif
	}
}