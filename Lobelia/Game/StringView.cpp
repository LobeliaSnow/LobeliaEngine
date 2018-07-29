#include "Common/Common.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config/Config.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/DisplayInfo/DisplayInfo.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/SwapChain/SwapChain.hpp"
#include "Graphics/Direct2D/Direct2DSystem.hpp"
#include "Scene/Scene.hpp"
#include "Application/Application.hpp"
#include "Game/StringView.hpp"
#include <mbstring.h>

namespace Lobelia::Game {
	//ê‹ÇËï‘ÇµÇ‡é¿ëï
	StringView::StringView(const char* font_name, int size, float interval_time, const std::string& str) :font(std::make_unique<Lobelia::Graphics::Font>(font_name, size)), timer(0.0f), nowLength(0), intervalTime(interval_time) {
		SetString(str);
		CalcStringByte();
	}
	void StringView::Format(int enter_count) {
		CalcStringByte();
		int byteCount = 0;
		int count = 0;
		str.resize(str.size() + 20);
		for (int i = 0; i < charCount; i++) {
			if (bytes[i] == 1 && str[byteCount] == '\n')continue;
			byteCount += bytes[i];
			count++;
			if (count >= enter_count) {
				str.insert(byteCount, "\n");
				bytes.insert(std::next(bytes.begin(), i), 1);
				charCount++;
				count = 0;
			}
		}
	}
	void StringView::CalcStringByte() {
		const char* pointer = str.c_str();
		charCount = 0;
		bytes.clear();
		bytes.resize(str.length() + 20);
		while (*pointer != 0) {
			bytes[charCount] = _mbclen(r_cast<const unsigned char*>(pointer));
			pointer += bytes[charCount];
			charCount++;
		}
	}
	int StringView::StrLengthByteCount(int length) {
		CalcStringByte();
		int ret = 0;
		for (int i = 0; i < length; i++) {
			ret += bytes[i];
		}
		return ret;
	}
	void StringView::SetString(const std::string& str) {
		this->str = str;
		timer = 0.0f;
		nowLength = 0;
		CalcStringByte();
	}
	void StringView::AddString(const std::string& str) {
		this->str += "\n" + str;
		CalcStringByte();
	}
	void StringView::SetMax() { nowLength = charCount; }
	bool StringView::IsEnd() { return (nowLength >= charCount); }
	void StringView::Update() {
		timer += Lobelia::Application::GetInstance()->GetProcessTimeMili();
		if (timer > intervalTime) {
			timer -= intervalTime;
			nowLength++;
		}
		if (nowLength >= charCount)nowLength = charCount;
	}
	void StringView::Render(const Math::Vector2& pos) {
		std::string renderString = str.substr(0, StrLengthByteCount(nowLength));
		Utility::Color color;
		color.a = 255; color.r = 100; color.g = 250; color.b = 255;
		Graphics::Direct2DRenderer::ColorChange(color);
		//R100 G250 B255
		Lobelia::Graphics::Direct2DRenderer::FontRender(font.get(), pos, renderString.c_str());
		Graphics::Direct2DRenderer::ColorChange(0xFFFFFFFF);
	}

}