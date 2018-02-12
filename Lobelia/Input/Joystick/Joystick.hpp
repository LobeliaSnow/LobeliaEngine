#pragma once
#include <Xinput.h>
#pragma comment (lib, "xinput.lib")

namespace Lobelia::Input {
	class Joystick :public Utility::Singleton<Joystick> {
		friend class Utility::Singleton<Joystick>;
	private:
		static std::vector<DIDEVICEINSTANCE> deviceList;
		static int xInputDeviceCount;
	private:
		static bool IsXInputDevice(const GUID* guid_from_dinput);
		static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* inst, VOID* context);
	public:
		//定数ではないが実質定数なので大文字
		struct DirectInputPadSet {
			//軸
			int LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y;
			int A, B, X, Y;
			int L1, L2, L3;
			int R1, R2, R3;
			int START, SELECT;
		};
		//アクセス用キーコード
		enum class KeyCode { UP, DOWN, LEFT, RIGHT, A, B, X, Y, L1, L2, L3, R1, R2, R3, START, SELECT };
	private:
		//こいつが最終的にユーザーのアクセスする構造体
		//DirectInputとXInputの差異を吸収するためのもの
		struct Controller {
			bool isXinput;
			Math::Vector2 leftAxis;
			Math::Vector2 rightAxis;
			//ボタン
			std::array<BYTE, 20> button;
			bool isPushAnyKey;
			Controller(bool is_xinput);
		};
		class DirectInputController :public InputDevice {
		private:
			static BOOL CALLBACK EnumAxis(LPCDIDEVICEOBJECTINSTANCE inst, LPVOID ref);
		private:
			int asignMap[16];
			int pov[4];
		private:
			bool Poll();
		public:
			DirectInputController(HWND hwnd, const GUID& guid, const DIDATAFORMAT& format, bool fore_ground, bool exclusive);
			~DirectInputController() = default;
			void SetPadSet(const DirectInputPadSet& padset);
			void Update(Controller* data);
		};
	private:
		int dinputDeviceCount;
		std::vector<std::unique_ptr<DirectInputController>> dinputControllers;
		//前からDirectInput[]+XInput[]
		std::vector<Controller> controllers;
		DirectInputPadSet defaultPadSet;
		int controllerCount;
	private:
		void CreateDirectInputController(HWND hwnd, bool fore_ground, bool exclusive);
		void XInputUpdate();
		void CheckIndex(int index);
	public:
		void Initialize(HWND hwnd, bool fore_ground = true, bool exclusive = false);
		//XInputの時は無視されます -1の時はデフォルトに設定されます
		void SetDirectInputPadSet(int index, const DirectInputPadSet& padset);
		void Update();
		int GetControllerCount();
		BYTE GetKey(int index, KeyCode code);
		bool IsPushAnyKey(int index);
		const Math::Vector2& GetLeftAxis(int index);
		const Math::Vector2& GetRightAxis(int index);
	private:
		Joystick() = default;
		~Joystick() = default;
		Joystick(const Joystick&) = delete;
		Joystick(Joystick&&) = delete;
		Joystick& operator =(const Joystick&) = delete;
		Joystick& operator =(Joystick&&) = delete;
	};
}