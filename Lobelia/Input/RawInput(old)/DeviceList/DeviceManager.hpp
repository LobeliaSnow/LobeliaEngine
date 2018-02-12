#pragma once
namespace Lobelia::Input {
	class DeviceManager {
	private:
		static std::vector<MouseData> mouseData;
		static std::vector<KeyboardData> keyboardData;
		static std::vector<DualShock4Data> dualShock4Data;
	private:
		static std::unique_ptr<RAWINPUTDEVICELIST[]> deviceList;
		static UINT  deviceCount;
		static std::vector<std::pair<HANDLE, std::shared_ptr<Mouse>>> mouses;
		static UINT mouseCount;
		static std::vector<std::pair<HANDLE, std::shared_ptr<Keyboard>>> keyboards;
		static UINT keyboardCount;
		static std::vector<std::pair<HANDLE, std::shared_ptr<DualShock4>>> dualShock4s;
		static UINT dualShock4Count;

	private:
		static void UpdateMouse(RAWINPUT* raw);
		static void UpdateKeyboard(RAWINPUT* raw);
		static void UpdateDualShock4(RAWINPUT* raw);
		static void AddDualShock4(HANDLE device);
	public:
		static void TakeDevices(HWND hwnd);
		static void Clear();
		static LRESULT UpdateProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
		static void Update();
		static int GetMouseCount();
		static Mouse* GetMouse(int i);
		static int GetKeyboardCount();
		static Keyboard* GetKeyboard(int i); 
		static int GetDualShock4Count();
		static DualShock4* GetDualShock4(int i);
		template<class T> static BYTE GetKey(int key_code);
		static BYTE GetKeyboardKey(int key_code);
		static BYTE GetMouseKey(int key_code);
		static BYTE GetDualShock4Key(DualShock4::KeyCode key_code);

	};
	BYTE GetKeyboardKey(int key_code);
	BYTE GetMouseKey(int key_code);
}
#include "DeviceManager.inl"