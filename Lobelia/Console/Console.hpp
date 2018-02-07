#pragma once
namespace Lobelia {
	class HostConsole :public Utility::Singleton<HostConsole> {
		friend class Utility::Singleton<HostConsole>;
	private:
		class InformationConsole* information = nullptr;
		class LogConsole* logs = nullptr;
		class CommandConsole* commander = nullptr;
	private:
		HostConsole();
		~HostConsole();
		bool IsActive();
	public:
		enum class ExeStyle {
			ALWAYS,
			BUTTON,
		};
	public:
		void Bootup();
		void Shutdown();
		void StringRegister(const char* key, const char* label, char* data);
		void IntRegister(const char* key, const char* label, int* data, bool analyze);
		void FloatRegister(const char* key, const char* label, float* data, bool analyze);
		void Vector3Register(const char* key, const char* label, Math::Vector3* data, bool analyze);
		void Vector2Register(const char* key, const char* label, Math::Vector2* data, bool analyze);
		void VariableUnRegister(const char* key, const char* label);
		void VariableUnRegister(const char* key);
		void SetLog(const std::string& log);
		template<class... Args> void Printf(const char* format, Args... args) {
			char buffer[256] = {};
			sprintf_s(buffer, format, args...);
			SetLog(buffer);
		}
		bool CommandRegister(const char* cmd, ExeStyle style, const std::function<bool()>& exe);
		void CommandUnRegister(const char* cmd);
		void ProcessRegister(const char* key, const std::function<void()>& process);
		void ProcessUnRegister(const char* key);
		void ClearCommand();
		void UpdateAndRender();
		void UpdateProcess();
	};
}