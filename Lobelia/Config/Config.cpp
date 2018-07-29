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
		//アンチエイリアス
		preference.msaa.Count = s_cast<UINT>(xmlApplicationInfo["msaa"]["count"].GetInt());
		preference.msaa.Quality = s_cast<UINT>(xmlApplicationInfo["msaa"]["quality"].GetInt());
		//アプリ更新FPS
		preference.updateFPS = xmlApplicationInfo["fps"].GetFloat();
		//VSyncを使用する
		preference.useVSync = xmlApplicationInfo["vsync"].GetInt();
#ifdef USE_IMGUI_AND_CONSOLE
		auto& xmlConsoleInfo = xml->Root()["console"];
		auto& console = preference.consoleOption;
		//アクティブ状態
		console.active = (xmlConsoleInfo["active"].GetInt());
		//システム情報を表示するか否か
		console.systemVisible = (xmlConsoleInfo["sys_visible"].GetInt());
		//表示位置
		console.systemPos.x = xmlConsoleInfo["state_pos"]["x"].GetFloat();
		console.systemPos.y = xmlConsoleInfo["state_pos"]["y"].GetFloat();
		//表示サイズ
		console.systemSize.x = xmlConsoleInfo["state_size"]["x"].GetFloat();
		console.systemSize.y = xmlConsoleInfo["state_size"]["y"].GetFloat();
		//インフォメーションコンソールの表示
		console.informationVisible = (xmlConsoleInfo["info_visible"].GetInt());
		//ログコンソールの表示
		console.logVisible = (xmlConsoleInfo["log_visible"].GetInt());
		//コマンドコンソールの表示
		console.commandVisible = (xmlConsoleInfo["command_visible"].GetInt());
		//プロセス更新処理のアクティブ状態
		console.processUpdate = (xmlConsoleInfo["process_update"].GetInt());
		//変数監視のアクティブ状態
		console.variableAnalyze = (xmlConsoleInfo["var_analyze"].GetInt());
		//変数監視のインターバル
		console.variableAnalyzeDomain = xmlConsoleInfo["var_analyze_domain"].GetFloat();
		//メモリとCPU使用率の更新インターバル
		console.memoryCpuUsageDomain = xmlConsoleInfo["memory_cpu_usage_domain"].GetFloat();
		//インフォメーションコンソールの位置
		console.informationPos.x = xmlConsoleInfo["info_pos"]["x"].GetFloat();
		console.informationPos.y = xmlConsoleInfo["info_pos"]["y"].GetFloat();
		//インフォメーションコンソールのサイズ
		console.informationSize.x = xmlConsoleInfo["info_size"]["x"].GetFloat();
		console.informationSize.y = xmlConsoleInfo["info_size"]["y"].GetFloat();
		//ログコンソールの位置
		console.logPos.x = xmlConsoleInfo["log_pos"]["x"].GetFloat();
		console.logPos.y = xmlConsoleInfo["log_pos"]["y"].GetFloat();
		//ログコンソールのサイズ
		console.logSize.x = xmlConsoleInfo["log_size"]["x"].GetFloat();
		console.logSize.y = xmlConsoleInfo["log_size"]["y"].GetFloat();
		//コマンドコンソールの位置
		console.commandPos.x = xmlConsoleInfo["command_pos"]["x"].GetFloat();
		console.commandPos.y = xmlConsoleInfo["command_pos"]["y"].GetFloat();
		//コマンドコンソールのサイズ
		console.commandSize.x = xmlConsoleInfo["command_size"]["x"].GetFloat();
		console.commandSize.y = xmlConsoleInfo["command_size"]["y"].GetFloat();
		//先頭ログの色
		console.logFirstColor.a = 255;
		console.logFirstColor.r = xmlConsoleInfo["log_first_color"]["red"].GetInt();
		console.logFirstColor.g = xmlConsoleInfo["log_first_color"]["green"].GetInt();
		console.logFirstColor.b = xmlConsoleInfo["log_first_color"]["blue"].GetInt();
		//そのほかのログの色
		console.logOtherColor.a = 255;
		console.logOtherColor.r = xmlConsoleInfo["log_other_color"]["red"].GetInt();
		console.logOtherColor.g = xmlConsoleInfo["log_other_color"]["green"].GetInt();
		console.logOtherColor.b = xmlConsoleInfo["log_other_color"]["blue"].GetInt();
		//ログの保存件数
		console.logMax = xmlConsoleInfo["log_count"].GetInt();
#endif

	}
	Config::Preference& Config::GetRefPreference() { return preference; }

}