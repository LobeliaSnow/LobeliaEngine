#pragma once
namespace Lobelia::Game {
	//mbclen �g�����@�\����
	//���͍�蒼��
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
		//������𕶎����ɍ��킹���s���� ���� ���s������
		void Format(int enter_count);
	private:
		//����������o�C�g�����Z�o
		int StrLengthByteCount(int length);
	private:
		std::string str;
		std::vector<int> bytes;
		std::unique_ptr<Lobelia::Graphics::Font> font;
		//���ݓ��B���Ă��镶����
		int nowLength;
		//������
		int charCount;
		float timer;
		float intervalTime;
	};
}