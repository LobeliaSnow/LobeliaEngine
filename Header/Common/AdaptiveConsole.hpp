#pragma once
namespace Lobelia {
	//自由領域のImGuiウインドウを提供します
	class AdaptiveConsole {
	public:
		AdaptiveConsole(const char* window_tag);
		~AdaptiveConsole() = default;
		void AddFunction(const std::function<void()>& function);
		void Update();
	private:
		std::string windowTag;
		std::list<std::function<void()>> functor;
	};
}