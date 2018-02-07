#pragma once
namespace Lobelia::Game {
	class StringView {
	private:
		std::string str;
		std::unique_ptr<Lobelia::Graphics::Font> font;
		int size;
		int length;
		float timer;
		float intervalTime;
	public:
		StringView(const char* font_name, int size, float interval_time, const std::string& str) :font(std::make_unique<Lobelia::Graphics::Font>(font_name, size)), timer(0.0f), length(0), intervalTime(interval_time) {
			SetString(str);
		}
		void SetString(const std::string& str) {
			this->str = str;
			size = i_cast(str.size());
			timer = 0.0f;
		}
		void AddString(const std::string& str) {
			timer = ((size) / 2) * intervalTime;
			this->str += " \n" + str;
			size = i_cast(this->str.size());
		}
		bool isEnd() { return (i_cast(timer / intervalTime) * 2 >= size); }
		void Render(const Math::Vector2& pos) {
			length = i_cast(timer / intervalTime) * 2;
			//if (length >= size)length = size;
			timer += Lobelia::Application::GetInstance()->GetProcessTime();
			if (timer > (size / 2) * intervalTime)timer = (size / 2) * intervalTime;
			Lobelia::Graphics::Direct2DRenderer::FontRender(font.get(), pos, str.substr(0, length).c_str());
		}
	};
}