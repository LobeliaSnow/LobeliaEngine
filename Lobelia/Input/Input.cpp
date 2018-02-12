#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Input.hpp"

namespace Lobelia::Input {
	void DirectInput::Initialize() {
		HRESULT hr = S_OK;
		hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, r_cast<void**>(device.GetAddressOf()), nullptr);
		if (FAILED(hr))STRICT_THROW("DirectInput�f�o�C�X�̍쐬�Ɏ��s");
	}
	ComPtr<IDirectInput8>& DirectInput::GetDevice() { return device; }
	void InputDevice::Initialize(HWND hwnd, const GUID& guid, const DIDATAFORMAT& format, bool fore_ground, bool exclusive) {
		decltype(auto) device = DirectInput::GetInstance()->GetDevice();
		HRESULT hr = S_OK;
		hr = device->CreateDevice(guid, inputDevice.GetAddressOf(), nullptr);
		if (FAILED(hr))STRICT_THROW("���̓f�o�C�X�̍쐬�Ɏ��s");
		hr = inputDevice->SetDataFormat(&format);
		if (FAILED(hr))STRICT_THROW("���̓f�o�C�X�̃f�[�^�ݒ�Ɏ��s");
		//�������x���̐ݒ�
		DWORD cooperativeLevel = 0;
		if (fore_ground) cooperativeLevel |= DISCL_FOREGROUND;
		else cooperativeLevel |= DISCL_BACKGROUND;
		if (exclusive) cooperativeLevel |= DISCL_EXCLUSIVE;
		else cooperativeLevel |= DISCL_NONEXCLUSIVE;
		hr = inputDevice->SetCooperativeLevel(hwnd, cooperativeLevel);
		if (FAILED(hr))STRICT_THROW("�������x���̐ݒ�Ɏ��s");
	}
	bool InputDevice::Acquire() {
		HRESULT hr = inputDevice->Acquire();
		if (FAILED(hr))return false;
		return true;
	}
	ComPtr<IDirectInputDevice8>& InputDevice::GetDevice() { return inputDevice; }

}