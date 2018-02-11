#include "Common/Common.hpp"
#include "Input/DeviceList/Device.hpp"
#include "Input/Data/Data.hpp"
#include "Input/Mouse/Mouse.hpp"
#include "Input/Keyboard/Keyboard.hpp"
#include "Input/Controller//DS4/DS4.hpp"
#include "Input/DeviceList/DeviceManager.hpp"
#include "Exception/Exception.hpp"
#include "Console/Console.hpp"

namespace Lobelia::Input {
	std::vector<MouseData> DeviceManager::mouseData;
	std::vector<KeyboardData> DeviceManager::keyboardData;
	std::vector<DualShock4Data> DeviceManager::dualShock4Data;

	std::unique_ptr<RAWINPUTDEVICELIST[]> DeviceManager::deviceList;
	UINT  DeviceManager::deviceCount = 0;
	std::vector<std::pair<HANDLE, std::shared_ptr<Mouse>>> DeviceManager::mouses;
	UINT DeviceManager::mouseCount = 0;
	std::vector<std::pair<HANDLE, std::shared_ptr<Keyboard>>> DeviceManager::keyboards;
	UINT DeviceManager::keyboardCount = 0;
	std::vector<std::pair<HANDLE, std::shared_ptr<DualShock4>>> DeviceManager::dualShock4s;
	UINT DeviceManager::dualShock4Count = 0;

	void DeviceManager::TakeDevices(HWND hwnd) {
		Clear();
		GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST));
		if (deviceCount <= 0)STRICT_THROW("接続されているデバイスがありません");
		deviceList = std::make_unique<RAWINPUTDEVICELIST[]>(deviceCount);
		if (GetRawInputDeviceList(deviceList.get(), &deviceCount, sizeof(RAWINPUTDEVICELIST)) != deviceCount)STRICT_THROW("デバイス取得失敗");
		for (int i = 0; i < static_cast<int>(deviceCount); i++) {
			UINT size = 256;
			RID_DEVICE_INFO info;
			char name[256] = {};
			GetRawInputDeviceInfo(deviceList[i].hDevice, RIDI_DEVICENAME, name, &size);
			size = sizeof(info);
			GetRawInputDeviceInfo(deviceList[i].hDevice, RIDI_DEVICEINFO, &info, &size);

			switch (deviceList[i].dwType) {
			case RIM_TYPEMOUSE:
				mouses.push_back(std::make_pair(deviceList[i].hDevice, std::make_shared<Mouse>(deviceList[i].hDevice)));
				mouseCount++;
				break;
			case RIM_TYPEKEYBOARD:
				keyboards.push_back(std::make_pair(deviceList[i].hDevice, std::make_shared<Keyboard>(deviceList[i].hDevice)));
				keyboardCount++;
				break;
			case RIM_TYPEHID:
				break;
			default:	break;
			}
		}
		mouseData.resize(mouseCount);
		keyboardData.resize(keyboardCount);
		RAWINPUTDEVICE device[4];
		device[0].usUsagePage = 0x01;
		device[0].usUsage = 0x02;		// マウス用の定数
		device[0].dwFlags = 0;
		device[0].hwndTarget = hwnd;
		device[1].usUsagePage = 0x01;
		device[1].usUsage = 0x06;		// キーボード用の定数
		device[1].dwFlags = 0;
		device[1].hwndTarget = hwnd;
		device[2].usUsagePage = 0x01;
		device[2].usUsage = 0x05;		//コントローラー用の定数
		device[2].dwFlags = 0;
		device[2].hwndTarget = hwnd;
		device[3].usUsagePage = 0x01;
		device[3].usUsage = 0x04;		//ジョイスティック用の定数
		device[3].dwFlags = 0;
		device[3].hwndTarget = hwnd;

		if (!RegisterRawInputDevices(device, 4, sizeof(RAWINPUTDEVICE)))	STRICT_THROW("デバイスの登録に失敗");
	}
	void DeviceManager::Clear() {
		mouseData.clear();
		deviceList.reset();
		deviceCount = 0;
		mouses.clear();
		mouseCount = 0;
	}
	void DeviceManager::UpdateMouse(RAWINPUT* raw) {
		int i = 0;
		for each(auto&& mouse in mouses) {
			if (mouse.first == raw->header.hDevice) {
				if (raw->data.mouse.ulButtons == RI_MOUSE_LEFT_BUTTON_DOWN)mouseData[i].buffer[0] = true;
				else if (raw->data.mouse.ulButtons == RI_MOUSE_LEFT_BUTTON_UP)mouseData[i].buffer[0] = false;
				else if (raw->data.mouse.ulButtons == RI_MOUSE_MIDDLE_BUTTON_DOWN)mouseData[i].buffer[1] = true;
				else if (raw->data.mouse.ulButtons == RI_MOUSE_MIDDLE_BUTTON_UP)mouseData[i].buffer[1] = false;
				else if (raw->data.mouse.ulButtons == RI_MOUSE_RIGHT_BUTTON_DOWN)mouseData[i].buffer[2] = true;
				else if (raw->data.mouse.ulButtons == RI_MOUSE_RIGHT_BUTTON_UP)mouseData[i].buffer[2] = false;
				if (raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL)mouseData[i].wheel = raw->data.mouse.usButtonData;
				Math::Vector2 move;
				mouseData[i].move.x += raw->data.mouse.lLastX;
				mouseData[i].move.y += raw->data.mouse.lLastY;
			}
			i++;
		}
	}
	void DeviceManager::UpdateKeyboard(RAWINPUT* raw) {
		int i = 0;
		for each(auto& keyboard in keyboards) {
			if (keyboard.first == raw->header.hDevice) {
				if (raw->data.keyboard.Flags == RI_KEY_MAKE)keyboardData[i].buffer[raw->data.keyboard.VKey] = true;
				else if (raw->data.keyboard.Flags == RI_KEY_BREAK)keyboardData[i].buffer[raw->data.keyboard.VKey] = false;
				else if (raw->data.keyboard.Flags == RI_KEY_E0)keyboardData[i].buffer[raw->data.keyboard.VKey] = true;
				else if (raw->data.keyboard.Flags == RI_KEY_E1)keyboardData[i].buffer[raw->data.keyboard.VKey] = true;
				else keyboardData[i].buffer[raw->data.keyboard.VKey] = false;
			}
			i++;
		}
	}
	void DeviceManager::UpdateDualShock4(RAWINPUT* raw) {
		UINT size = raw->data.hid.dwSizeHid;
		if (size != 64UL)STRICT_THROW("DualShock4でないはずのコントローラーがDualShock4と認識されました");
		int i = 0;
		for each(auto& ds4 in dualShock4s) {
			if (ds4.first == raw->header.hDevice) {
				dualShock4Data[i].buffer[DualShock4::KeyCode::CIRCLE] |= CIRCLE_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::CROSS] |= CROSS_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::SQUARE] |= SQUARE_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::TRIANGLE] |= TRIANGLE_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::R3] |= R3_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::L3] |= L3_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::OPTION] |= OPTION_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::SHARE] |= SHARE_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::R1] |= R1_CHECK(raw->data.hid.bRawData);
				dualShock4Data[i].buffer[DualShock4::KeyCode::L1] |= L1_CHECK(raw->data.hid.bRawData);
				//十字キー分解取得				
				if (AXIS_UP_CHECK(raw->data.hid.bRawData))dualShock4Data[i].buffer[DualShock4::KeyCode::UP] = true;
				else if (AXIS_DOWN_CHECK(raw->data.hid.bRawData))dualShock4Data[i].buffer[DualShock4::KeyCode::DOWN] = true;
				else if (AXIS_LEFT_CHECK(raw->data.hid.bRawData))dualShock4Data[i].buffer[DualShock4::KeyCode::LEFT] = true;
				else if (AXIS_RIGHT_CHECK(raw->data.hid.bRawData))dualShock4Data[i].buffer[DualShock4::KeyCode::RIGHT] = true;
				else if (AXIS_UPRIGHT_CHECK(raw->data.hid.bRawData)) {
					dualShock4Data[i].buffer[DualShock4::KeyCode::UP] |= true;
					dualShock4Data[i].buffer[DualShock4::KeyCode::RIGHT] |= true;
				}
				else if (AXIS_RIGHTDOWN_CHECK(raw->data.hid.bRawData)) {
					dualShock4Data[i].buffer[DualShock4::KeyCode::RIGHT] |= true;
					dualShock4Data[i].buffer[DualShock4::KeyCode::DOWN] |= true;
				}
				else if (AXIS_DOWNLEFT_CHECK(raw->data.hid.bRawData)) {
					dualShock4Data[i].buffer[DualShock4::KeyCode::DOWN] |= true;
					dualShock4Data[i].buffer[DualShock4::KeyCode::LEFT] |= true;
				}
				else if (AXIS_LEFTUP_CHECK(raw->data.hid.bRawData)) {
					dualShock4Data[i].buffer[DualShock4::KeyCode::LEFT] |= true;
					dualShock4Data[i].buffer[DualShock4::KeyCode::UP] |= true;
				}
				break;
			}
			i++;
		}
	}
	void DeviceManager::AddDualShock4(HANDLE device) {
		for each(auto ds4 in dualShock4s) {
			if (ds4.first == device)return;
		}
		dualShock4s.push_back(std::make_pair(device, std::make_shared<DualShock4>(device)));
		dualShock4Data.push_back({});
		dualShock4Count++;
	}

	//最終的にここに入ってきたデバイスをデバイスリストに突っ込んでやる形式に持っていく
	//RawInputのデバイスリストは解雇
	LRESULT DeviceManager::UpdateProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		if (msg == WM_INPUT) {
			UINT size;
			GetRawInputData((HRAWINPUT)lp, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
			std::unique_ptr<unsigned char[]> data = std::make_unique<unsigned char[]>(size);
			if (!data)STRICT_THROW("");
			if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, data.get(), &size, sizeof(RAWINPUTHEADER)) != size)STRICT_THROW("入力情報の取得の失敗");
			PRAWINPUT raw = (RAWINPUT*)data.get();
			RID_DEVICE_INFO info = {};
			info.cbSize = sizeof(RID_DEVICE_INFO);
			//RIDI_DEVICEINFO
			TCHAR devName[256] = {};
			size = sizeof(devName);
			GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, devName, &size);
			size = info.cbSize;
			GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICEINFO, &info, &size);
			switch (raw->header.dwType) {
			case RIM_TYPEMOUSE:			UpdateMouse(raw);		break;
			case RIM_TYPEKEYBOARD:	UpdateKeyboard(raw);	break;
			case RIM_TYPEHID:
				if (!DualShock4::IsDualShock4(info))break;
				AddDualShock4(raw->header.hDevice);
				UpdateDualShock4(raw);
				break;
			}
			//UINT nInput = GetRawInputBuffer(raw, &size, sizeof(RAWINPUTHEADER));
			//DefRawInputProc(&raw, nInput, sizeof(RAWINPUTHEADER));
		}
		return 0;
	}
	void DeviceManager::Update() {
		for (int i = 0; i < static_cast<int>(mouseCount); i++) {
			mouses[i].second->Update(mouseData[i]);
			mouseData[i].move = {};
			mouseData[i].wheel = 0;
		}
		for (int i = 0; i < static_cast<int>(keyboardCount); i++) {
			keyboards[i].second->Update(keyboardData[i]);
		}
		for (int i = 0; i < i_cast(dualShock4Count); i++) {
			dualShock4s[i].second->Update(dualShock4Data[i]);
			dualShock4Data[i] = {};
		}
		WindowsMousePointer::GetInstance()->Update();
	}
	int DeviceManager::GetMouseCount() { return mouseCount; }
	Mouse* DeviceManager::GetMouse(int i) { return mouses[i].second.get(); }
	int DeviceManager::GetKeyboardCount() { return keyboardCount; }
	Keyboard* DeviceManager::GetKeyboard(int i) { return keyboards[i].second.get(); }
	int DeviceManager::GetDualShock4Count() { return dualShock4Count; }
	DualShock4* DeviceManager::GetDualShock4(int i) { return dualShock4s[i].second.get(); }
	BYTE DeviceManager::GetKeyboardKey(int key_code) { return GetKey<Keyboard>(key_code); }
	BYTE DeviceManager::GetMouseKey(int key_code) { return GetKey<Mouse>(key_code); }
	BYTE DeviceManager::GetDualShock4Key(DualShock4::KeyCode key_code) { return GetKey<DualShock4>(key_code); }

	BYTE GetKeyboardKey(int key_code) {
		return DeviceManager::GetKeyboardKey(key_code);
	}
	BYTE GetMouseKey(int key_code) {
		return DeviceManager::GetMouseKey(key_code);
	}
}