#include "Lobelia.hpp"
#include "Common/AdaptiveConsole.hpp"

namespace Lobelia {
	AdaptiveConsole::AdaptiveConsole(const char* window_tag) :windowTag(window_tag) {
	}
	void AdaptiveConsole::AddFunction(const std::function<void()>& function) { functor.push_back(function); }
	void AdaptiveConsole::Update() {
#ifdef USE_IMGUI_AND_CONSOLE
		ImGui::Begin(windowTag.c_str());
		for (auto&& function : functor) {
			function();
		}
		ImGui::End();
#endif
	}
}