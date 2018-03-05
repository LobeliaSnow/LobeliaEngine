#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Input/Input.hpp"
#include "Keyboard.hpp"

namespace Lobelia::Input {
	Keyboard::Keyboard() :buffer{}, pushKeyNo(-1){}
	void Keyboard::Initialize(HWND hwnd, bool fore_ground, bool exclusive) {
		InputDevice::Initialize(hwnd, GUID_SysKeyboard, c_dfDIKeyboard, fore_ground, exclusive);
	}
	void Keyboard::Update() {
		BYTE state[256] = {};
		HRESULT hr = S_OK;
		if (Acquire()) {
			hr = GetDevice()->GetDeviceState(sizeof(state), &state);
			if (FAILED(hr))STRICT_THROW("�f�o�C�X�̏�Ԃ̎擾�Ɏ��s");
		}
		pushKeyNo = -1;
		for (int i = 0; i < 256; i++) {
			buffer[i] <<= 1;
			buffer[i] |= (state[i] != 0);
			if (pushKeyNo == -1 && buffer[i] == 1)pushKeyNo = i;
		}
	}
	BYTE Keyboard::GetKey(int key_code) {
		if (s_cast<UINT>(key_code) > 255)STRICT_THROW("�͈͊O�̃L�[�R�[�h���ݒ肳��܂���");
		return buffer[key_code] & 3;
	}
	bool Keyboard::IsPushAnyKey() { return (pushKeyNo != -1); }
	int Keyboard::PushKeyNo() { return pushKeyNo; }
	BYTE GetKeyboardKey(int key_code) { return Keyboard::GetInstance()->GetKey(key_code); }
}
