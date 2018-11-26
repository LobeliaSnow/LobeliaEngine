#pragma once
namespace Lobelia {
	class HostConsole :public Utility::Singleton<HostConsole> {
		friend class Utility::Singleton<HostConsole>;
	private:
#ifdef USE_IMGUI_AND_CONSOLE
		std::unique_ptr<class InformationConsole> information;
		std::unique_ptr<class LogConsole> logs;
		std::unique_ptr<class CommandConsole> commander;
		std::unique_ptr<class SystemConsole> system;
#endif
	private:
		//�R���X�g���N�^�ƃf�X�g���N�^����C�����C���ł���K�v������
		//unique_ptr�͐������Ɣj���̎��ɃN���X�̊��S�^��v�����邪�A
		//�C�����C���ɂ��đO���錾�ōs���Ă��܂��ƁA���S�^�����Ȃ�
		//������̉�����Ƃ��ẮA�J�X�^���f���[�^�[�Ő��䂷�邱��
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
		void SaveLog(const char* file_path);
		void SetProcessTime(float process_time);
		template<class... Args> void Printf(const char* format, Args&&... args) {
			char buffer[256] = {};
			sprintf_s(buffer, format, std::forward<Args>(args)...);
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