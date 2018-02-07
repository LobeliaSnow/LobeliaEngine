#pragma once
namespace Lobelia {
	class Config {
		using Stage = Graphics::ShaderStageList;
	private:
		struct Preference {
			//システムの定数バッファがアクティブになるステージ
			byte systemCBActiveStage = Stage::VS | Stage::GS | Stage::PS;
			DXGI_SAMPLE_DESC msaa = { 1,0 };
			struct ApplicationOption {
				bool systemVisible = true;
				int updateFPS = 60;
				Math::Vector2 pos = Math::Vector2(0, 0);
				Math::Vector2 size = Math::Vector2(200, 120);
			}applicationOption;
#ifdef USE_IMGUI_AND_CONSOLE
			struct HostConsoleOption {
				//ウインドウを動かせるようにするか否かの設定も必要かな
				//サイズ感に関しては現状直値だけどウインドウサイズからの割合に変更すること
				bool active = true;
				bool informationVisible = true;
				bool logVisible = true;
				bool commandVisible = true;
				bool processUpdate = true;
				bool variableAnalyze = true;
				float variableAnalyzeDomain = 1.0f;
				Math::Vector2 informationPos = Math::Vector2(0, 120);
				Math::Vector2 informationSize = Math::Vector2(200, 200);
				Math::Vector2 logPos = Math::Vector2(640, 620);
				Math::Vector2 logSize = Math::Vector2(640, 100);
				Math::Vector2 commandPos = Math::Vector2(0, 320);
				Math::Vector2 commandSize = Math::Vector2(200, 200);
				Utility::Color logFirstColor = 0xFF00FF00;
				Utility::Color logOtherColor = 0xFFFFFFFF;
				int logMax = 100;
			}consoleOption;
#endif
		};
	private:
		static Preference preference;
	public:
		static Preference& GetRefPreference();
	};
}