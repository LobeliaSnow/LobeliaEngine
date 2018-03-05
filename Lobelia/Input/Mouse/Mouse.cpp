#include "Lobelia.hpp"

namespace Lobelia::Input {
	Mouse::Mouse() :buffer{}, move(), wheel(0), pushKeyNo(-1){}
	void Mouse::Initialize(HWND hwnd, bool fore_ground, bool exclusive) {
		InputDevice::Initialize(hwnd, GUID_SysMouse, c_dfDIMouse2, fore_ground, exclusive);
		//軸モードを相対値モードに設定
		DIPROPDWORD property = {};
		property.diph.dwSize = sizeof(DIPROPDWORD);
		property.diph.dwHeaderSize = sizeof(property.diph);
		property.diph.dwObj = 0;
		property.diph.dwHow = DIPH_DEVICE;
		property.dwData = DIPROPAXISMODE_REL;
		//絶対値モードの場合
		//property.dwData = DIPROPAXISMODE_ABS;
		HRESULT hr = GetDevice()->SetProperty(DIPROP_AXISMODE, &property.diph);
		if (FAILED(hr))STRICT_THROW("プロパティの設定に失敗");
	}
	void Mouse::Update() {
		DIMOUSESTATE2 state = {};
		if (Acquire()) {
			HRESULT hr = GetDevice()->GetDeviceState(sizeof(DIMOUSESTATE2), &state);
			if (FAILED(hr))STRICT_THROW("入力デバイスの情報取得に失敗");
		}
		pushKeyNo = -1;
		for (int i = 0; i < 3; i++) {
			buffer[i] <<= 1;
			buffer[i] |= (state.rgbButtons[i] != 0);
			if (pushKeyNo == -1 && buffer[i])pushKeyNo = i;
		}
		move.x = f_cast(state.lX);
		move.y = f_cast(state.lY);
		wheel = state.lZ;
		//マウス座標取得
		POINT mpos = {};
		GetCursorPos(&mpos);
		screenPos.x = f_cast(mpos.x);
		screenPos.y = f_cast(mpos.y);
		ScreenToClient(Application::GetInstance()->GetWindow()->GetHandle(), &mpos);
		clientPos.x = f_cast(mpos.x);
		clientPos.y = f_cast(mpos.y);
	}
	BYTE Mouse::GetKey(int key_code) {
		if (s_cast<UINT>(key_code) > 3)STRICT_THROW("範囲外のキーコードが設定されました");
		return buffer[key_code] & 3;
	}
	bool Mouse::IsPushAnyKey() { return (pushKeyNo != -1); }
	int Mouse::PushKeyNo() { return pushKeyNo; }
	const Math::Vector2& Mouse::GetMove() { return move; }
	int Mouse::GetWheel() { return wheel; }
	const Math::Vector2& Mouse::GetClientPos() { return clientPos; }
	const Math::Vector2& Mouse::GetScreenPos() { return screenPos; }

	BYTE GetMouseKey(int key_code) { return Mouse::GetInstance()->GetKey(key_code); }

}