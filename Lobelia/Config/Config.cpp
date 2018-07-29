#include "Common/Common.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config.hpp"
#include "Exception/Exception.hpp"
#include "XML/XML.hpp"

namespace Lobelia {
	Config::Preference Config::preference;

	void Config::LoadSetting(const char* file_path) {
		std::unique_ptr<XMLParser> xml = std::make_unique<XMLParser>(file_path);
		auto& xmlApplicationInfo = xml->Root()["application"];
		//�A���`�G�C���A�X
		preference.msaa.Count = s_cast<UINT>(xmlApplicationInfo["msaa"]["count"].GetInt());
		preference.msaa.Quality = s_cast<UINT>(xmlApplicationInfo["msaa"]["quality"].GetInt());
		//�A�v���X�VFPS
		preference.updateFPS = xmlApplicationInfo["fps"].GetFloat();
		//VSync���g�p����
		preference.useVSync = xmlApplicationInfo["vsync"].GetInt();
#ifdef USE_IMGUI_AND_CONSOLE
		auto& xmlConsoleInfo = xml->Root()["console"];
		auto& console = preference.consoleOption;
		//�A�N�e�B�u���
		console.active = (xmlConsoleInfo["active"].GetInt());
		//�V�X�e������\�����邩�ۂ�
		console.systemVisible = (xmlConsoleInfo["sys_visible"].GetInt());
		//�\���ʒu
		console.systemPos.x = xmlConsoleInfo["state_pos"]["x"].GetFloat();
		console.systemPos.y = xmlConsoleInfo["state_pos"]["y"].GetFloat();
		//�\���T�C�Y
		console.systemSize.x = xmlConsoleInfo["state_size"]["x"].GetFloat();
		console.systemSize.y = xmlConsoleInfo["state_size"]["y"].GetFloat();
		//�C���t�H���[�V�����R���\�[���̕\��
		console.informationVisible = (xmlConsoleInfo["info_visible"].GetInt());
		//���O�R���\�[���̕\��
		console.logVisible = (xmlConsoleInfo["log_visible"].GetInt());
		//�R�}���h�R���\�[���̕\��
		console.commandVisible = (xmlConsoleInfo["command_visible"].GetInt());
		//�v���Z�X�X�V�����̃A�N�e�B�u���
		console.processUpdate = (xmlConsoleInfo["process_update"].GetInt());
		//�ϐ��Ď��̃A�N�e�B�u���
		console.variableAnalyze = (xmlConsoleInfo["var_analyze"].GetInt());
		//�ϐ��Ď��̃C���^�[�o��
		console.variableAnalyzeDomain = xmlConsoleInfo["var_analyze_domain"].GetFloat();
		//��������CPU�g�p���̍X�V�C���^�[�o��
		console.memoryCpuUsageDomain = xmlConsoleInfo["memory_cpu_usage_domain"].GetFloat();
		//�C���t�H���[�V�����R���\�[���̈ʒu
		console.informationPos.x = xmlConsoleInfo["info_pos"]["x"].GetFloat();
		console.informationPos.y = xmlConsoleInfo["info_pos"]["y"].GetFloat();
		//�C���t�H���[�V�����R���\�[���̃T�C�Y
		console.informationSize.x = xmlConsoleInfo["info_size"]["x"].GetFloat();
		console.informationSize.y = xmlConsoleInfo["info_size"]["y"].GetFloat();
		//���O�R���\�[���̈ʒu
		console.logPos.x = xmlConsoleInfo["log_pos"]["x"].GetFloat();
		console.logPos.y = xmlConsoleInfo["log_pos"]["y"].GetFloat();
		//���O�R���\�[���̃T�C�Y
		console.logSize.x = xmlConsoleInfo["log_size"]["x"].GetFloat();
		console.logSize.y = xmlConsoleInfo["log_size"]["y"].GetFloat();
		//�R�}���h�R���\�[���̈ʒu
		console.commandPos.x = xmlConsoleInfo["command_pos"]["x"].GetFloat();
		console.commandPos.y = xmlConsoleInfo["command_pos"]["y"].GetFloat();
		//�R�}���h�R���\�[���̃T�C�Y
		console.commandSize.x = xmlConsoleInfo["command_size"]["x"].GetFloat();
		console.commandSize.y = xmlConsoleInfo["command_size"]["y"].GetFloat();
		//�擪���O�̐F
		console.logFirstColor.a = 255;
		console.logFirstColor.r = xmlConsoleInfo["log_first_color"]["red"].GetInt();
		console.logFirstColor.g = xmlConsoleInfo["log_first_color"]["green"].GetInt();
		console.logFirstColor.b = xmlConsoleInfo["log_first_color"]["blue"].GetInt();
		//���̂ق��̃��O�̐F
		console.logOtherColor.a = 255;
		console.logOtherColor.r = xmlConsoleInfo["log_other_color"]["red"].GetInt();
		console.logOtherColor.g = xmlConsoleInfo["log_other_color"]["green"].GetInt();
		console.logOtherColor.b = xmlConsoleInfo["log_other_color"]["blue"].GetInt();
		//���O�̕ۑ�����
		console.logMax = xmlConsoleInfo["log_count"].GetInt();
#endif

	}
	Config::Preference& Config::GetRefPreference() { return preference; }

}