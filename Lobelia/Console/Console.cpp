#include "Common/Common.hpp"
#include "Console.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config/Config.hpp"
#include "Scene/Scene.hpp"
#include "Application/Application.hpp"
#include "Exception/Exception.hpp"

//TODO : �֐������A�y�эœK��
//��
//�o�^���ꂽ�f�[�^���Ď����āA�ύX���������u�ԂɃ��O�ɋL�^
//command�̂��낢��ǉ�
//���̂ق��ɂ����O�̊O���ۑ��₻�̑��������ǉ��\��
//�~�����Ă��W���܂��B

namespace Lobelia {
#ifdef USE_IMGUI_AND_CONSOLE
	class InformationConsole {
	private:
		enum Attribute :int {
			STRING,
			INT,
			FLOAT
		};
		struct InformationData {
			union {
				int oldIData;
				float oldFData;
			};
			union {
				char* sData;
				int* iData;
				float* fData;
			};
			char label[32];
			Attribute attribute;
			bool analyze;
		};
	private:
		std::map<std::string, std::list<InformationData>> dataList;
		char objectTable[5096] = {};//��������������ƍH�v���K�v�A�������󋵂ɉ����ĕς���Ȃ�
		std::vector<std::string> keyMap;
		std::string name;
		Math::Vector2 pos;
		Math::Vector2 size;
		int selectItem;
		float counter;
		bool addTable;
	private:
		bool AddTable(const char* key);
		void TableFormat();
		void Register(const char* key, const char* label, void* data, Attribute attribute, bool analyze);
		void BeginRender();
		void InformationRender();
		void EndRender();
	public:
		//�E�C���h�E���󂯎�����ق����ǂ������A���O���̋���ɂ�茻�󕡐��ɂ��ē��s��
		//��ʒu�ƃT�C�Y�󂯎��
		InformationConsole(const char* name, const Math::Vector2& pos, const Math::Vector2& size);
		~InformationConsole();
		//Regist�n�͎Q�ƃ`�F�b�N���Ă��Ȃ��̂ŎQ�Ɛ悪�����Ȃ��悤�ɋC��t���邱��
		//����L�[�̒���label���Ԃ肪�N����Ƃ����Ȃ��̂ŋN���Ȃ��悤�ɋC��t���邱��
		//�߂�l ���O
		std::string StringRegister(const char* key, const char* label, char* data);
		std::string IntRegister(const char* key, const char* label, int* data, bool analyze);
		std::string FloatRegister(const char* key, const char* label, float* data, bool analyze);
		std::string UnRegister(const char* key, const char* label);
		std::string UnRegister(const char* key);
		void SetupAnalaysis();
		void Analaysis(std::string* log);
		void UpdateAndRender();
	};
	//�O���t�@�C���ɏo�͂���@�\�����Ă�������
	class LogConsole {
	private:
		std::list<std::string> logs;
		std::string name;
		Math::Vector2 pos;
		Math::Vector2 size;
	private:
		void LogLimitCheck();
	public:
		LogConsole(const char* name, const Math::Vector2& pos, const Math::Vector2& size);
		~LogConsole();
		void SetLog(const std::string& log);
		void Clear();
		void UpdateAndRender();
	};
	//���͒���v���郏�[�h�����Ƃ��ĕ\������̂���
	class CommandConsole {
	private:
		char command[64];
		std::string name;
		Math::Vector2 pos;
		Math::Vector2 size;
		std::map<std::string, std::pair<HostConsole::ExeStyle, std::function<bool()>>> executor;
		HostConsole::ExeStyle style;
		std::map<std::string, std::function<void()>> process;
	private:
		bool ExecuteCommand(const std::pair<HostConsole::ExeStyle, std::function<bool()>>& exe);
	public:
		CommandConsole(const char* name, const Math::Vector2& pos, const Math::Vector2& size);
		~CommandConsole();
		void UpdateProcess();
		bool CommandRegister(const char* cmd, HostConsole::ExeStyle style, const std::function<bool()>& exe);
		void CommandUnRegister(const char* cmd);
		void ProcessRegister(const char* key, const std::function<void()>& process);
		void ProcessUnRegister(const char* key);
		void ClearCommandBox();
		std::string UpdateAndRender();
	};
	InformationConsole::InformationConsole(const char* name, const Math::Vector2& pos, const Math::Vector2& size) :name(name), pos(pos), size(size), selectItem(0), counter(0), addTable(false) {}
	InformationConsole::~InformationConsole() {
	}
	bool InformationConsole::AddTable(const char* key) {
		if (this->dataList.find(key) == this->dataList.end()) {
			keyMap.push_back(key);
			addTable = true;
			return true;
		}
		return false;
	}
	void InformationConsole::TableFormat() {
		if (!addTable)return;
		strcpy_s(objectTable, "");
		int keyCount = s_cast<int>(keyMap.size());
		for (int i = 0; i < keyCount; i++) {
			strcat_s(objectTable, keyMap[i].c_str());
			strcat_s(objectTable, "+");
		}
		int size = static_cast<int>(strnlen_s(objectTable, 5096));
		for (int i = 0; i < size; i++) {
			if (objectTable[i] == '+')objectTable[i] = '\0';
		}
		addTable = false;
	}

	//�]�T������΃��x���̏d���`�F�b�N���Ă��̃��O��Ԃ�
	void InformationConsole::Register(const char* key, const char* label, void* data, Attribute attribute, bool analyze) {
		InformationData renderTransform;
		renderTransform.attribute = attribute;
		strcpy_s(renderTransform.label, label);
		switch (renderTransform.attribute) {
		case Attribute::STRING:	renderTransform.sData = static_cast<char*>(data);	break;
		case Attribute::INT:
			renderTransform.iData = static_cast<int*>(data);
			renderTransform.oldIData = *renderTransform.iData;
			break;
		case Attribute::FLOAT:
			renderTransform.fData = static_cast<float*>(data);
			renderTransform.oldFData = *renderTransform.fData;
			break;
		default:STRICT_THROW("���m�̑����ł�");	break;
		}
		renderTransform.analyze = analyze;
		dataList[key].push_back(renderTransform);
	}
	std::string InformationConsole::StringRegister(const char* key, const char* label, char* data) {
		std::string log;
		if (AddTable(key))log += "Add Key ->" + std::string(key);
		Register(key, label, data, Attribute::STRING, false);
		return log += " " + std::string(label) + " ->" + static_cast<char*>(data);
	}
	std::string InformationConsole::IntRegister(const char* key, const char* label, int* data, bool analyze) {
		std::string log;
		if (AddTable(key))log += "Add Key ->" + std::string(key);
		Register(key, label, data, Attribute::INT, analyze);
		return log += " " + std::string(label) + " ->" + std::to_string(static_cast<int>(*data));
	}
	std::string InformationConsole::FloatRegister(const char* key, const char* label, float* data, bool analyze) {
		std::string log;
		if (AddTable(key))log += "Add Key ->" + std::string(key);
		Register(key, label, data, Attribute::FLOAT, analyze);
		return log += " " + std::string(label) + " ->" + std::to_string(static_cast<float>(*data));
	}
	std::string InformationConsole::UnRegister(const char* key, const char* label) {
		auto& now = dataList.find(key);
		if (now != dataList.end()) {
			for (auto& d = now->second.begin(); d != now->second.end();) {
				if (strcmp(d->label, label) == 0)now->second.erase(d);
			}
		}
		return "deleted data ->key " + std::string(key) + " ->label " + label;
	}
	std::string InformationConsole::UnRegister(const char* key) {
		//�֐��������ق������������H
		auto& it0 = dataList.find(key);
		if (it0 != dataList.end())	dataList.erase(it0);
		int keyCount = s_cast<int>(keyMap.size());
		for (auto commpareKey = keyMap.begin(); commpareKey != keyMap.end();) {
			if (*commpareKey == key)commpareKey = keyMap.erase(commpareKey);
			else commpareKey++;
		}
		return "deleted data ->key " + std::string(key);
	}

	void InformationConsole::BeginRender() {
		//�v����
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_::ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		std::string renderTransform = "��" + name;//���{�ꂪ������ƕ\������Ȃ����ۂ𐶂����ĕ\�����������Ȃ�������Ȃ̂Ŕ�\����
		ImGui::Combo(renderTransform.c_str(), &selectItem, objectTable, 5);
	}
	void InformationConsole::InformationRender() {
		TableFormat();
		if (dataList.empty())return;
		int i = 0;
		if (selectItem >= keyMap.size())selectItem = 0;
		for (auto& data = dataList[keyMap[selectItem]].begin(); data != dataList[keyMap[selectItem]].end(); data++) {
			if (data->attribute == Attribute::STRING) {
				std::string str = "label-" + std::string(data->label) + " data-" + data->sData;
				ImGui::Text(str.c_str());
			}
			if (data->attribute == Attribute::INT) {
				int compare = *data->iData;
				ImGui::InputInt(data->label, data->iData);
			}
			if (data->attribute == Attribute::FLOAT) {
				float compare = *data->fData;
				ImGui::InputFloat(data->label, data->fData);
			}
			if (data->attribute != Attribute::STRING&&ImGui::Button(std::string(std::string("Analyze") + std::to_string(i) + (data->analyze ? " ON" : " OFF")).c_str()))data->analyze = !data->analyze;
			i++;
		}
	}
	void InformationConsole::EndRender() { ImGui::End(); }
	void InformationConsole::SetupAnalaysis() {
		for (auto& data = dataList.begin(); data != dataList.end(); data++) {
			for (auto& source = data->second.begin(); source != data->second.end(); source++) {
				switch (source->attribute) {
				case Attribute::STRING:break;
				case Attribute::INT:			source->oldIData = *source->iData; break;
				case Attribute::FLOAT:	source->oldFData = *source->fData; break;
				default:	break;
				}
			}
		}
	}
	void InformationConsole::Analaysis(std::string* log) {
		if (counter < Config::GetRefPreference().consoleOption.variableAnalyzeDomain)return;
		counter = 0;
		for (auto& data = dataList.begin(); data != dataList.end(); data++) {
			for (auto& source = data->second.begin(); source != data->second.end(); source++) {
				if (!source->analyze)continue;
				switch (source->attribute) {
				case Attribute::STRING:break;
				case Attribute::INT:
					if (*source->iData != source->oldIData) {
						*log += "data changed ->key-" + data->first + " label-" + std::string(source->label) + " value " + std::to_string(source->oldIData) + " ->" + std::to_string(*source->iData) + "\n";
						source->oldIData = *source->iData;
					}
				case Attribute::FLOAT:
					if (*source->fData != source->oldFData) {
						*log += "data changed ->key-" + data->first + " label-" + std::string(source->label) + " value " + std::to_string(source->oldFData) + " ->" + std::to_string(*source->fData) + "\n";
						source->oldFData = *source->fData;
					}
				default:		break;
				}
			}
		}
	}

	void InformationConsole::UpdateAndRender() {
		counter += Application::GetInstance()->GetProcessTime() / 1000.0f;
		BeginRender();
		InformationRender();
		EndRender();
	}

	LogConsole::LogConsole(const char* name, const Math::Vector2& pos, const Math::Vector2& size) :name(name), pos(pos), size(size) {

	}
	LogConsole::~LogConsole() = default;
	void LogConsole::LogLimitCheck() { while (logs.size() > Config::GetRefPreference().consoleOption.logMax)logs.pop_back(); }
	void LogConsole::SetLog(const std::string& log) {
		logs.push_front(log);
		LogLimitCheck();
	}
	void LogConsole::Clear() { logs.clear(); }
	void LogConsole::UpdateAndRender() {
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_::ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		bool first = true;
		for each(auto&& log in logs) {
			if (first) {
				Utility::Color color = Config::GetRefPreference().consoleOption.logFirstColor;
				ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(color.r, color.g, color.b, color.a);
				first = false;
			}
			else {
				Utility::Color color = Config::GetRefPreference().consoleOption.logOtherColor;
				ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(color.r, color.g, color.b, color.a);
			}
			ImGui::Text(log.c_str());
		}
		ImGui::End();
		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	CommandConsole::CommandConsole(const char* name, const Math::Vector2& pos, const Math::Vector2& size) :name(name), pos(pos), size(size), command("") {
		CommandRegister("change fps", HostConsole::ExeStyle::ALWAYS, [&]() {
			static int fps = Config::GetRefPreference().applicationOption.updateFPS;
			ImGui::InputInt("", &fps);
			if (ImGui::Button("command execute")) {
				Config::GetRefPreference().applicationOption.updateFPS = fps;
				return true;
			}
			return false;
		});
		CommandRegister("change log count", HostConsole::ExeStyle::ALWAYS, [&]() {
			static int log = Config::GetRefPreference().consoleOption.logMax;
			ImGui::InputInt("", &log);
			if (ImGui::Button("command execute")) {
				Config::GetRefPreference().consoleOption.logMax = log;
				//ClearCommandBox();
				return true;
			}
			return false;
		});

	}
	CommandConsole::~CommandConsole() = default;
	void CommandConsole::UpdateProcess() {
		for each(auto& p in process) {
			p.second();
		}
	}
	bool CommandConsole::ExecuteCommand(const std::pair<HostConsole::ExeStyle, std::function<bool()>>& exe) {
		//�����Ƀ��p�[�g���[�𑝂₷�B
		switch (exe.first) {
		case HostConsole::ExeStyle::BUTTON:
			if (ImGui::Button("command execute")) return exe.second();
			break;
		case HostConsole::ExeStyle::ALWAYS:
			return exe.second();
			break;
		default:	break;
		}
		return false;
	}
	bool CommandConsole::CommandRegister(const char* cmd, HostConsole::ExeStyle style, const std::function<bool()>& exe) {
		if (executor.find(cmd) != executor.end())return false;
		executor[cmd] = std::make_pair(style, exe);
		return true;
	}
	void CommandConsole::CommandUnRegister(const char* cmd) {
		if (executor.find(cmd) == executor.end())return;
		executor.erase(cmd);
	}
	void CommandConsole::ProcessRegister(const char* key, const std::function<void()>& process) {
		if (executor.find(key) != executor.end())return;
		this->process[key] = process;
	}
	void CommandConsole::ProcessUnRegister(const char* key) {
		if (executor.find(key) == executor.end())return;
		this->process.erase(key);
	}
	void CommandConsole::ClearCommandBox() { strcpy_s(command, ""); };
	std::string CommandConsole::UpdateAndRender() {
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_::ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		std::string renderTransform = "��" + name;
		ImGui::InputText(renderTransform.c_str(), command, 64);
		std::string ret;
		for each(auto& exe in executor) {
			if (exe.first == command) {
				if (ExecuteCommand(exe.second))ret = exe.first;
			}
			else {
				//�⊮
				//TODO : �������啶����ʂ��Ȃ��悤�ɂ���B
				if (exe.first.find(command) != std::string::npos) {
					if (ImGui::Button(exe.first.c_str()))strcpy_s(command, exe.first.c_str());
				}
			}
		}
		ImGui::End();
		return ret;
	}
#endif

	HostConsole::HostConsole() = default;
	HostConsole::~HostConsole() = default;

	bool HostConsole::IsActive() { 
#ifdef USE_IMGUI_AND_CONSOLE
		return Config::GetRefPreference().consoleOption.active; 
#else
		return false;
#endif
	}

	void HostConsole::Bootup() {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.ChildWindowRounding = 0.0f;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 1.0f, 0.7f);
		auto& option = Config::GetRefPreference().consoleOption;
		Utility::SafeNew(&information, "Information Console", option.informationPos, option.informationSize);
		Utility::SafeNew(&logs, "Log Console", option.logPos, option.logSize);
		Utility::SafeNew(&commander, "Command Console", option.commandPos, option.commandSize);
		commander->CommandRegister("clear log", HostConsole::ExeStyle::BUTTON, [this]() {logs->Clear(); return true; });
		information->SetupAnalaysis();
#endif
	}
	void HostConsole::Shutdown() {
#ifdef USE_IMGUI_AND_CONSOLE
		Utility::SafeDelete(information);
		Utility::SafeDelete(logs);
		Utility::SafeDelete(commander);
#endif
	}
	void HostConsole::StringRegister(const char* key, const char* label, char* data) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string log = information->StringRegister(key, label, data);
		if (!log.empty())SetLog(log);
#endif
	}
	void HostConsole::IntRegister(const char* key, const char* label, int* data, bool analyze) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string log = information->IntRegister(key, label, data, analyze);
		if (!log.empty())SetLog(log);
#endif
	}
	void HostConsole::FloatRegister(const char* key, const char* label, float* data, bool analyze) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string log = information->FloatRegister(key, label, data, analyze);
		if (!log.empty())SetLog(log);
#endif
	}
	void HostConsole::Vector3Register(const char* key, const char* label, Math::Vector3* data, bool analyze) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string labelString = label;
		std::string log = information->FloatRegister(key, (labelString + " x").c_str(), &data->x, analyze) + "\n";
		log += information->FloatRegister(key, (labelString + " y").c_str(), &data->y, analyze) + "\n";
		log += information->FloatRegister(key, (labelString + " z").c_str(), &data->z, analyze);
		if (!log.empty())SetLog(log);
#endif
	}
	void HostConsole::Vector2Register(const char* key, const char* label, Math::Vector2* data, bool analyze) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string labelString = label;
		std::string log = information->FloatRegister(key, (labelString + " x").c_str(), &data->x, analyze) + "\n";
		log += information->FloatRegister(key, (labelString + " y").c_str(), &data->y, analyze) + "\n";
		if (!log.empty())SetLog(log);
#endif
	}
	void HostConsole::VariableUnRegister(const char* key, const char* label) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string log = information->UnRegister(key, label);
		if (!log.empty())SetLog(log);
#endif
	}
	void HostConsole::VariableUnRegister(const char* key) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		std::string log = information->UnRegister(key);
		if (!log.empty())SetLog(log);
#endif
	}

	void HostConsole::SetLog(const std::string& log) {
#ifdef USE_IMGUI_AND_CONSOLE
		logs->SetLog(log); 
#endif
	}
	bool HostConsole::CommandRegister(const char* cmd, ExeStyle style, const std::function<bool()>& exe) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return false;
		return commander->CommandRegister(cmd, style, exe);
#endif
		return false;
	}
	void HostConsole::CommandUnRegister(const char* cmd) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		commander->CommandUnRegister(cmd);
#endif
	}
	void HostConsole::ProcessRegister(const char* key, const std::function<void()>& process) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		commander->ProcessRegister(key, process);
#endif
	}
	void HostConsole::ProcessUnRegister(const char* key) {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		commander->ProcessUnRegister(key);
#endif
	}
	void HostConsole::ClearCommand() {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		commander->ClearCommandBox();
#endif
	}
	void HostConsole::UpdateAndRender() {
#ifdef USE_IMGUI_AND_CONSOLE
		if (!IsActive())return;
		auto& option = Config::GetRefPreference().consoleOption;
		if (option.informationVisible)information->UpdateAndRender();
		if (option.logVisible)logs->UpdateAndRender();
		if (option.commandVisible) {
			std::string c = commander->UpdateAndRender();
			if (!c.empty())SetLog("command execute " + c);
		}
		std::string log;
		if (option.variableAnalyze)information->Analaysis(&log);
		if (!log.empty())logs->SetLog(log);
#endif
	}
	void HostConsole::UpdateProcess() { 
#ifdef USE_IMGUI_AND_CONSOLE
		if (Config::GetRefPreference().consoleOption.processUpdate)commander->UpdateProcess();
#endif
	}
}