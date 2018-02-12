#pragma once
namespace Lobelia::Input {
	//�ǂ����\���L�[�����Ă��Ȃ��H
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
