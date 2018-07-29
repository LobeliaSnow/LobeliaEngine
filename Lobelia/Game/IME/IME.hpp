#pragma once
namespace Lobelia::Game {
	class IME :public Utility::Singleton<IME> {
		friend class Utility::Singleton<IME>;
	public:
		enum class ConfigMode {
			GENERAL = IME_CONFIG_GENERAL,
			REGISTER_WORLD = IME_CONFIG_REGISTERWORD,
			SELECT_DICTIONARY = IME_CONFIG_SELECTDICTIONARY,
		};
	public:
		void Open(bool open);
		bool IsOpen();
		bool IsIME();
		//単語登録モードの場合のみ第二引数が必要
		void OpenConfig(ConfigMode mode, void* data = nullptr);
		//テスト場
		void Func();
		void Procedure(UINT msg, WPARAM wp, LPARAM lp);
	private:
		IME();
		~IME();
		HKL GetKeyboardLocale();

	private:
		HIMC context;
		std::string inputBox;
	};
}