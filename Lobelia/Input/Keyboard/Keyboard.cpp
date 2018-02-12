#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Input/Input.hpp"
#include "Keyboard.hpp"

namespace Lobelia::Input {
	Keyboard::Keyboard() :buffer{}, isPushAnyKey(false){}
	void Keyboard::Initialize(HWND hwnd, bool fore_ground, bool exclusive) {
		InputDevice::Initialize(hwnd, GUID_SysKeyboard, c_dfDIKeyboard, fore_ground, exclusive);
	}
	void Keyboard::Update() {
		BYTE state[256] = {};
		HRESULT hr = S_OK;
		if (Acquire()) {
			hr = GetDevice()->GetDeviceState(sizeof(state), &state);
			if (FAILED(hr))STRICT_THROW("デバイスの状態の取得に失敗");
		}
		isPushAnyKey = false;
		for (int i = 0; i < 256; i++) {
			buffer[i] <<= 1;
			buffer[i] |= (state[i] != 0);
			if (buffer[i] == 1)isPushAnyKey = true;
		}
	}
	BYTE Keyboard::GetKey(int key_code) {
		if (s_cast<UINT>(key_code) > 255)STRICT_THROW("範囲外のキーコードが設定されました");
		return buffer[key_code] & 3;
	}
	bool Keyboard::IsPushAnyKey() { return isPushAnyKey; }
	BYTE GetKeyboardKey(int key_code) { return Keyboard::GetInstance()->GetKey(key_code); }
}
