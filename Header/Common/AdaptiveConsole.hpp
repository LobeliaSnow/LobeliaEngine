#pragma once
namespace Lobelia {
	//���R�̈��ImGui�E�C���h�E��񋟂��܂�
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