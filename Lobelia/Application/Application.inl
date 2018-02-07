#pragma once
namespace Lobelia {
	template<class Scene, class ...Args> void Application::ChangeSceneReserve(Args... args) {
		if (tempScene)return;
		tempScene = new Scene(args...);
		changeScene = true;
	}
	template<class Scene, class ...Args> void Application::Bootup(const Math::Vector2& size, const char* window_name, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)>wnd_proc, Args... args) {
		window = std::make_shared<Window>(size, window_name, wnd_proc, WS_OVERLAPPEDWINDOW);
		swapChain = std::make_unique<Graphics::SwapChain>(window.get(), Config::GetRefPreference().msaa);
		ChangeSceneReserve<Scene>(args...);
		ChangeSceneExecute();
		window->ShowWindow(SW_SHOW);
		window->UppdateWindow();
		timer->Begin();
		Input::DeviceManager::TakeDevices(window->GetHandle());
		Graphics::Direct2DRenderer::Initialize();
		//swapChain->Get()->SetFullscreenState(true, nullptr);
#ifdef USE_IMGUI_AND_CONSOLE
		ImGui_ImplDX11_Init(window->GetHandle(), Graphics::Device::Get().Get(), Graphics::Device::GetContext().Get());
		HostConsole::GetInstance()->ProcessRegister("command clear", [=]() {if (Input::DeviceManager::GetKey<Input::Keyboard>(VK_F2) == 1)HostConsole::GetInstance()->ClearCommand(); });
		HostConsole::GetInstance()->ProcessRegister("console change visible", [=]() {
			if (Input::DeviceManager::GetKey<Input::Keyboard>(VK_F1) == 1) {
				Config::GetRefPreference().consoleOption.active = !Config::GetRefPreference().consoleOption.active;
				Config::GetRefPreference().applicationOption.systemVisible = !Config::GetRefPreference().applicationOption.systemVisible;
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