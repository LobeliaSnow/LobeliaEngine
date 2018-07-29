#pragma once
namespace Lobelia::Game {
	//mbclen 使った機能向上
	//又は作り直し
	class StringView {
	public:
		StringView(const char* font_name, int size, float interval_time, const std::string& str);
		void CalcStringByte();
		void SetString(const std::string& str);
		void AddString(const std::string& str);
		bool IsEnd();
		void SetMax();
		void Update();
		void Render(const Math::Vector2& pos);
		//文字列を文字数に合わせ改行する 引数 改行文字数
		void Format(int enter_count);
	private:
		//文字数からバイト数を算出
		int StrLengthByteCount(int length);
	private:
		std::string str;
		std::vector<int> bytes;
		std::unique_ptr<Lobelia::Graphics::Font> font;
		//現在到達している文字数
		int nowLength;
		//文字数
		int charCount;
		float timer;
		float intervalTime;
	};
}