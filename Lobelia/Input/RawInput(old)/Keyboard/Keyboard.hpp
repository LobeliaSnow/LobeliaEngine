#pragma once
namespace Lobelia::Input {
	//どうやら十字キーが取れていない？
	class Keyboard :public Device {
	private:
		KeyboardData data;
	public:
		Keyboard(HANDLE handle);
		~Keyboard();
		void Update(const KeyboardData& data);
		int GetKey(int key_code);
	};
}
