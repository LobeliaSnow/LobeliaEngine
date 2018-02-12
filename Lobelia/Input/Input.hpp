#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")

namespace Lobelia::Input {
	//デバイスを所持
	class DirectInput :public Utility::Singleton<DirectInput> {
		friend class Utility::Singleton<DirectInput>;
	private:
		ComPtr<IDirectInput8> device;
	public:
		void Initialize();
		ComPtr<IDirectInput8>& GetDevice();
	private:
		DirectInput() = default;
		~DirectInput() = default;
		DirectInput(const DirectInput&) = delete;
		DirectInput(DirectInput&&) = delete;
		DirectInput& operator =(const DirectInput&) = delete;
		DirectInput& operator =(DirectInput&&) = delete;
	};
	class InputDevice {
	private:
		ComPtr<IDirectInputDevice8> inputDevice;
	protected:
		ComPtr<IDirectInputDevice8>& GetDevice();
		void Initialize(HWND hwnd, const GUID& guid, const DIDATAFORMAT& format, bool fore_ground, bool exclusive);
		//アクセス権取得
		bool Acquire();
	protected:
		InputDevice() = default;
		virtual ~InputDevice() = default;
	};
	BYTE GetKeyboardKey(int key_code);
	BYTE GetMouseKey(int key_code);

}