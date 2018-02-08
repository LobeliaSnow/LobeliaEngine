#pragma once
namespace Lobelia {
	class Config {
		using Stage = Graphics::ShaderStageList;
	private:
		struct Preference {
			//�V�X�e���̒萔�o�b�t�@���A�N�e�B�u�ɂȂ�X�e�[�W
			byte systemCBActiveStage = Stage::VS | Stage::GS | Stage::PS;
			DXGI_SAMPLE_DESC msaa;
			struct ApplicationOption {
				bool systemVisible;
				float updateFPS;
				Math::Vector2 pos;
				Math::Vector2 size;
			}applicationOption;
#ifdef USE_IMGUI_AND_CONSOLE
			struct HostConsoleOption {
				//�E�C���h�E�𓮂�����悤�ɂ��邩�ۂ��̐ݒ���K�v����
				//�T�C�Y���Ɋւ��Ă͌��󒼒l�����ǃE�C���h�E�T�C�Y����̊����ɕύX���邱��
				bool active;
				bool informationVisible;
				bool logVisible;
				bool commandVisible;
				bool processUpdate;
				bool variableAnalyze;
				float variableAnalyzeDomain;
				Math::Vector2 informationPos;
				Math::Vector2 informationSize;
				Math::Vector2 logPos;
				Math::Vector2 logSize;
				Math::Vector2 commandPos;
				Math::Vector2 commandSize;
				Utility::Color logFirstColor;
				Utility::Color logOtherColor;
				int logMax;
			}consoleOption;
#endif
		};
	private:
		static Preference preference;
	public:
		static void LoadSetting(const char* file_path);
		static Preference& GetRefPreference();
	};
}