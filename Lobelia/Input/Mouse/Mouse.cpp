#include "Lobelia.hpp"

namespace Lobelia::Input {
	Mouse::Mouse() :buffer{}, move(), wheel(0), isPushAnyKey(false){}
	void Mouse::Initialize(HWND hwnd, bool fore_ground, bool exclusive) {
		InputDevice::Initialize(hwnd, GUID_SysMouse, c_dfDIMouse2, fore_ground, exclusive);
		//�����[�h�𑊑Βl���[�h�ɐݒ�
		DIPROPDWORD property = {};
		property.diph.dwSize = sizeof(DIPROPDWORD);
		property.diph.dwHeaderSize = sizeof(property.diph);
		property.diph.dwObj = 0;
		property.diph.dwHow = DIPH_DEVICE;
		property.dwData = DIPROPAXISMODE_REL;
		//��Βl���[�h�̏ꍇ
		//property.dwData = DIPROPAXISMODE_ABS;
		HRESULT hr = GetDevice()->SetProperty(DIPROP_AXISMODE, &property.diph);
		if (FAILED(hr))STRICT_THROW("�v���p�e�B�̐ݒ�Ɏ��s");
	}
	void Mouse::Update() {
		DIMOUSESTATE2 state = {};
		if (Acquire()) {
			HRESULT hr = GetDevice()->GetDeviceState(sizeof(DIMOUSESTATE2), &state);
			if (FAILED(hr))STRICT_THROW("���̓f�o�C�X�̏��擾�Ɏ��s");
		}
		isPushAnyKey = false;
		for (int i = 0; i < 3; i++) {
			buffer[i] <<= 1;
			buffer[i] |= (state.rgbButtons[i] != 0);
			if (buffer[i])isPushAnyKey = true;
		}
		move.x = f_cast(state.lX);
		move.y = f_cast(state.lY);
		wheel = state.lZ;
		//�}�E�X���W�擾
		POINT mpos = {};
		GetCursorPos(&mpos);
		screenPos.x = f_cast(mpos.x);
		screenPos.y = f_cast(mpos.y);
		ScreenToClient(Application::GetInstance()->GetWindow()->GetHandle(), &mpos);
		clientPos.x = f_cast(mpos.x);
		clientPos.y = f_cast(mpos.y);
	}
	BYTE Mouse::GetKey(int key_code) {
		if (s_cast<UINT>(key_code) > 3)STRICT_THROW("�͈͊O�̃L�[�R�[�h���ݒ肳��܂���");
		return buffer[key_code] & 3;
	}
	bool Mouse::IsPushAnyKey() { return isPushAnyKey; }
	const Math::Vector2& Mouse::GetMove() { return move; }
	int Mouse::GetWheel() { return wheel; }
	const Math::Vector2& Mouse::GetClientPos() { return clientPos; }
	const Math::Vector2& Mouse::GetScreenPos() { return screenPos; }

	BYTE GetMouseKey(int key_code) { return Mouse::GetInstance()->GetKey(key_code); }

}