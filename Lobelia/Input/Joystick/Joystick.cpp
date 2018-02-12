#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Input/Input.hpp"
#include "Joystick.hpp"
#include <wbemidl.h>
#include <oleauto.h>

//TODO : DirectInputEffectを使用して、振動等を付ける
//TODO : 上が対応でき次第XInputでの実装

namespace Lobelia::Input {
	std::vector<DIDEVICEINSTANCE> Joystick::deviceList;
	int Joystick::xInputDeviceCount = 0;
	//超ブラックボックス https://msdn.microsoft.com/ja-jp/library/bb173051(v=vs.85).aspx
	//そのデバイスがXInput対応なのか調べる
	bool Joystick::IsXInputDevice(const GUID* guid_from_dinput) {
		BSTR sysNameSpace = nullptr;
		BSTR sysClassName = nullptr;
		BSTR sysDeviceID = nullptr;
		bool isXinputDevice = false;
		auto FreeBSTR = [&]()->void {
			if (sysNameSpace)SysFreeString(sysNameSpace);
			if (sysDeviceID)SysFreeString(sysDeviceID);
			if (sysClassName)SysFreeString(sysClassName);
		};
#define RELEASE_THROW throw 1
		try {//何かあればthrowで解放処理へ飛ばす
			ComPtr<IWbemLocator> locator;
			HRESULT hr = CoCreateInstance(__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)locator.GetAddressOf());
			if (FAILED(hr) || !locator)RELEASE_THROW;
			sysNameSpace = SysAllocString(L"\\\\.\\root\\cimv2");
			sysClassName = SysAllocString(L"Win32_PNPEntity");
			sysDeviceID = SysAllocString(L"DeviceID");
			if (!sysNameSpace || !sysClassName || !sysDeviceID)RELEASE_THROW;
			ComPtr<IWbemServices> services;
			hr = locator->ConnectServer(sysNameSpace, nullptr, nullptr, 0, 0, nullptr, nullptr, services.GetAddressOf());
			if (FAILED(hr))RELEASE_THROW;
			hr = CoSetProxyBlanket(services.Get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
			if (FAILED(hr))RELEASE_THROW;
			ComPtr<IEnumWbemClassObject> enumDevices;
			hr = services->CreateInstanceEnum(sysClassName, 0, nullptr, enumDevices.GetAddressOf());
			if (FAILED(hr) || !enumDevices)RELEASE_THROW;
			ComPtr<IWbemClassObject> devices[20];
			IWbemClassObject* storage[20] = {};
			DWORD returned = 0;
			while (true) {
				hr = enumDevices->Next(10000, 20, storage, &returned);
				//ComPtrへ移譲
				for (int i = 0; i < returned; i++) { devices[i] = storage[i]; }
				if (FAILED(hr) || returned == 0)RELEASE_THROW;
				for (int i = 0; i < i_cast(returned); i++) {
					VARIANT var;
					hr = devices[i]->Get(sysDeviceID, 0, &var, nullptr, nullptr);
					if (FAILED(hr) || var.vt != VT_BSTR || var.bstrVal == nullptr) continue;
					if (!wcsstr(var.bstrVal, L"IG_"))continue;
					DWORD pid = 0, vid = 0;
					WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
					if (strVid && swscanf_s(strVid, L"VID_%4X", &vid) != 1)vid = 0;
					WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
					if (strPid && swscanf_s(strPid, L"PID_%4X", &pid) != 1)pid = 0;
					DWORD vidPid = MAKELONG(vid, pid);
					if (vidPid == guid_from_dinput->Data1) {
						isXinputDevice = true;
						RELEASE_THROW;
					}
				}
			}
		}
		catch (...) {}
		//解放処理
		FreeBSTR();
		return isXinputDevice;
	}
	BOOL CALLBACK Joystick::EnumJoysticksCallback(const DIDEVICEINSTANCE* inst, VOID* context) {
		//XInputデバイスだった場合無視する
		if (IsXInputDevice(&inst->guidProduct)) xInputDeviceCount++;
		else {
			//デバイスの追加
			DIDEVICEINSTANCE temp = {};
			memcpy_s(&temp, sizeof(DIDEVICEINSTANCE), inst, sizeof(DIDEVICEINSTANCE));
			deviceList.push_back(std::move(temp));
		}
		return DIENUM_CONTINUE;
	}
	Joystick::Controller::Controller(bool is_xinput) :isXinput(is_xinput), button{} {}
	Joystick::DirectInputController::DirectInputController(HWND hwnd, const GUID& guid, const DIDATAFORMAT& format, bool fore_ground, bool exclusive) {
		InputDevice::Initialize(hwnd, guid, format, fore_ground, exclusive);
		//DIPROPDWORD property = {};
		//property.diph.dwSize = sizeof(DIPROPDWORD);
		//property.diph.dwHeaderSize = sizeof(property.diph);
		//property.diph.dwObj = 0;
		//property.diph.dwHow = DIPH_DEVICE;
		//property.dwData = DIPROPAUTOCENTER_OFF;
		//HRESULT hr = GetDevice()->SetProperty(DIPROP_AUTOCENTER, &property.diph);
		//if (FAILED(hr))STRICT_THROW("プロパティの設定に失敗");
		HRESULT hr = GetDevice()->EnumObjects(DirectInputController::EnumAxis, GetDevice().Get(), DIDFT_AXIS);
		if (FAILED(hr))STRICT_THROW("軸設定の失敗");
		Acquire();
	}
	BOOL CALLBACK Joystick::DirectInputController::EnumAxis(LPCDIDEVICEOBJECTINSTANCE inst, LPVOID ref) {
		DIPROPRANGE range = {};
		range.diph.dwSize = sizeof(DIPROPRANGE);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwObj = inst->dwType;
		range.diph.dwHow = DIPH_BYID;
		range.lMin = -1000;
		range.lMax = 1000;
		HRESULT hr = r_cast<LPDIRECTINPUTDEVICE8>(ref)->SetProperty(DIPROP_RANGE, &range.diph);
		if (FAILED(hr))STRICT_THROW("プロパティの設定に失敗");
	}
	bool Joystick::DirectInputController::Poll() {
		HRESULT hr = GetDevice()->Poll();
		if (FAILED(hr))return Acquire();
		return true;
	}
	void Joystick::DirectInputController::SetPadSet(const DirectInputPadSet& padset) {
		//左スティック
		asignMap[0] = padset.LEFT_X;
		asignMap[1] = padset.LEFT_Y;
		//右スティック
		asignMap[2] = padset.RIGHT_X;
		asignMap[3] = padset.RIGHT_Y;
		//メインボタン -1の理由はキー表は1~なのに対して、インデックスは0から始まるため
		asignMap[i_cast(KeyCode::A)] = padset.A - 1;
		asignMap[i_cast(KeyCode::B)] = padset.B - 1;
		asignMap[i_cast(KeyCode::X)] = padset.X - 1;
		asignMap[i_cast(KeyCode::Y)] = padset.Y - 1;
		//LR
		asignMap[i_cast(KeyCode::L1)] = padset.L1 - 1;
		asignMap[i_cast(KeyCode::L2)] = padset.L2 - 1;
		asignMap[i_cast(KeyCode::L3)] = padset.L3 - 1;
		asignMap[i_cast(KeyCode::R1)] = padset.R1 - 1;
		asignMap[i_cast(KeyCode::R2)] = padset.R2 - 1;
		asignMap[i_cast(KeyCode::R3)] = padset.R3 - 1;
		//スタート セレクト
		asignMap[i_cast(KeyCode::START)] = padset.START - 1;
		asignMap[i_cast(KeyCode::SELECT)] = padset.SELECT - 1;
	}
	void Joystick::DirectInputController::Update(Controller* data) {
		if (data->isXinput)STRICT_THROW("形式エラー");
		if (!Poll())return;
		DIJOYSTATE2 state;
		HRESULT hr = GetDevice()->GetDeviceState(sizeof(DIJOYSTATE2), &state);
		if (FAILED(hr))STRICT_THROW("デバイスの情報取得に失敗しました");
		//軸一時保存
		int axis[6] = {};
		axis[0] = state.lX;		axis[1] = state.lY;		axis[2] = state.lZ;
		axis[3] = state.lRx;	axis[4] = state.lRy;	axis[5] = state.lRz;
		//適用
		data->leftAxis.x = axis[asignMap[0]];
		data->leftAxis.y = axis[asignMap[1]];
		data->rightAxis.x = axis[asignMap[2]];
		data->rightAxis.y = axis[asignMap[3]];
		//十字キー判定
		int angle = 8;//何も押されていない状態を表す
		constexpr int POV_KEY[9] = { 0x01 ,0x09, 0x08, 0x0A, 0x02, 0x06, 0x04, 0x05, 0x00 };
		//8方向取得
		if (LOWORD(state.rgdwPOV[0]) != 0xFFFF)angle = state.rgdwPOV[0] / 4500;
		//適用
		data->isPushAnyKey = false;
		for (int direction = 0; direction < 4; direction++) {
			data->button[direction] <<= 1;
			data->button[direction] |= (POV_KEY[angle] & (0x01 << direction));
			if (data->button[direction] == 1)data->isPushAnyKey = true;
		}
		//ボタン
		for (int i = 4; i < 16; i++) {
			data->button[i] <<= 1;
			data->button[i] |= (state.rgbButtons[asignMap[i]] != 0);
			if (data->button[i] == 1)data->isPushAnyKey = true;
		}
	}

	void Joystick::Initialize(HWND hwnd, bool fore_ground, bool exclusive) {
		//デフォルトのキーアサイン作成(PS4準拠)
		defaultPadSet = DirectInputPadSet{ 0,1,2,5,2,3,1,4,7,5,11,8,6,12,9,10 };
		defaultPadSet.LEFT_X = 0;		defaultPadSet.LEFT_Y = 1;
		defaultPadSet.RIGHT_X = 2;	defaultPadSet.RIGHT_Y = 5;
		defaultPadSet.A = 3;				defaultPadSet.B = 2;
		defaultPadSet.X = 4;					defaultPadSet.Y = 1;
		defaultPadSet.L1 = 5;				defaultPadSet.L2 = 7;
		defaultPadSet.L3 = 11;				defaultPadSet.R1 = 6;
		defaultPadSet.R2 = 8;				defaultPadSet.R3 = 12;
		defaultPadSet.START = 10;		defaultPadSet.SELECT = 9;
		decltype(auto) device = DirectInput::GetInstance()->GetDevice();
		//全て初期化
		xInputDeviceCount = 0; deviceList.clear(); dinputControllers.clear(); controllers.clear(); controllerCount = 0;
		HRESULT hr = S_OK;
		hr = device->EnumDevices(DI8DEVCLASS_GAMECTRL, Joystick::EnumJoysticksCallback, nullptr, DIEDFL_ATTACHEDONLY);
		if (FAILED(hr))STRICT_THROW("DirectInput対応コントローラー検出に失敗");
		CreateDirectInputController(hwnd, fore_ground, exclusive);
		for (int i = 0; i < xInputDeviceCount; i++) {
			controllers.push_back(Controller(true));
		}
		controllerCount = controllers.size();
	}
	void Joystick::CreateDirectInputController(HWND hwnd, bool fore_ground, bool exclusive) {
		dinputDeviceCount = deviceList.size();
		dinputControllers.resize(dinputDeviceCount);
		for (int i = 0; i < dinputDeviceCount; i++) {
			dinputControllers[i] = std::make_unique<DirectInputController>(hwnd, deviceList[i].guidInstance, c_dfDIJoystick2, fore_ground, exclusive);
			dinputControllers[i]->SetPadSet(defaultPadSet);
			controllers.push_back(Controller(false));
		}
	}
	void Joystick::SetDirectInputPadSet(int index, const DirectInputPadSet& padset) {
		if (index >= dinputDeviceCount) return;
		dinputControllers[index]->SetPadSet(padset);
	}
	void Joystick::XInputUpdate() {
		auto Push = [=](BYTE* button, bool push) ->void {
			(*button) <<= 1;
			(*button) |= push;
		};
		for (int i = 0; i < xInputDeviceCount; i++) {
			XINPUT_STATE state = {};
			if (XInputGetState(i, &state) != ERROR_SUCCESS) {
				//コントローラーが切断されている
			}
			int index = i + dinputDeviceCount;
			auto& pad = state.Gamepad;
			Push(&controllers[index].button[i_cast(KeyCode::UP)], (pad.wButtons&XINPUT_GAMEPAD_DPAD_UP));
			Push(&controllers[index].button[i_cast(KeyCode::DOWN)], (pad.wButtons&XINPUT_GAMEPAD_DPAD_DOWN));
			Push(&controllers[index].button[i_cast(KeyCode::LEFT)], (pad.wButtons&XINPUT_GAMEPAD_DPAD_LEFT));
			Push(&controllers[index].button[i_cast(KeyCode::RIGHT)], (pad.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT));
			Push(&controllers[index].button[i_cast(KeyCode::A)], (pad.wButtons&XINPUT_GAMEPAD_A));
			Push(&controllers[index].button[i_cast(KeyCode::B)], (pad.wButtons&XINPUT_GAMEPAD_B));
			Push(&controllers[index].button[i_cast(KeyCode::X)], (pad.wButtons&XINPUT_GAMEPAD_X));
			Push(&controllers[index].button[i_cast(KeyCode::Y)], (pad.wButtons&XINPUT_GAMEPAD_Y));
			Push(&controllers[index].button[i_cast(KeyCode::L1)], (pad.wButtons&XINPUT_GAMEPAD_LEFT_SHOULDER));
			Push(&controllers[index].button[i_cast(KeyCode::L2)], (pad.bLeftTrigger));
			Push(&controllers[index].button[i_cast(KeyCode::L3)], (pad.wButtons&XINPUT_GAMEPAD_LEFT_THUMB));
			Push(&controllers[index].button[i_cast(KeyCode::R1)], (pad.wButtons&XINPUT_GAMEPAD_RIGHT_SHOULDER));
			Push(&controllers[index].button[i_cast(KeyCode::R2)], (pad.bRightTrigger));
			Push(&controllers[index].button[i_cast(KeyCode::R3)], (pad.wButtons&XINPUT_GAMEPAD_RIGHT_THUMB));
			controllers[index].leftAxis.x = f_cast(pad.sThumbLX) / 32768.0f*1000.0f;
			controllers[index].leftAxis.y = f_cast(pad.sThumbLY) / 32768.0f*1000.0f;
			controllers[index].rightAxis.x = f_cast(pad.sThumbRX) / 32768.0f*1000.0f;
			controllers[index].rightAxis.y = f_cast(pad.sThumbRY) / 32768.0f*1000.0f;
		}
	}
	void Joystick::Update() {
		int i = 0;
		for each(auto& controller in dinputControllers) {
			controller->Update(&controllers[i]);
			i++;
		}
		XInputUpdate();
	}
	void Joystick::CheckIndex(int index) {
		if (s_cast<UINT>(index) > s_cast<UINT>(controllerCount))STRICT_THROW("範囲外の値です");
	}
	int Joystick::GetControllerCount() { return controllerCount; }
	BYTE Joystick::GetKey(int index, KeyCode code) {
		CheckIndex(index);
		return controllers[index].button[i_cast(code)] & 3;
	}
	bool Joystick::IsPushAnyKey(int index) {
		CheckIndex(index);
		return controllers[index].isPushAnyKey;
	}
	const Math::Vector2& Joystick::GetLeftAxis(int index) {
		CheckIndex(index);
		return controllers[index].leftAxis;
	}
	const Math::Vector2& Joystick::GetRightAxis(int index) {
		CheckIndex(index);
		return controllers[index].rightAxis;
	}


}