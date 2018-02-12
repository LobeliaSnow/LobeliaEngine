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
		//�萔�ł͂Ȃ��������萔�Ȃ̂ő啶��
		struct DirectInputPadSet {
			//��
			int LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y;
			int A, B, X, Y;
			int L1, L2, L3;
			int R1, R2, R3;
			int START, SELECT;
		};
		//�A�N�Z�X�p�L�[�R�[�h
		enum class KeyCode { UP, DOWN, LEFT, RIGHT, A, B, X, Y, L1, L2, L3, R1, R2, R3, START, SELECT };
	private:
		//�������ŏI�I�Ƀ��[�U�[�̃A�N�Z�X����\����
		//DirectInput��XInput�̍��ق��z�����邽�߂̂���
		struct Controller {
			bool isXinput;
			Math::Vector2 leftAxis;
			Math::Vector2 rightAxis;
			//�{�^��
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
		//�O����DirectInput[]+XInput[]
		std::vector<Controller> controllers;
		DirectInputPadSet defaultPadSet;
		int controllerCount;
	private:
		void CreateDirectInputController(HWND hwnd, bool fore_ground, bool exclusive);
		void XInputUpdate();
		void CheckIndex(int index);
	public:
		void Initialize(HWND hwnd, bool fore_ground = true, bool exclusive = false);
		//XInput�̎��͖�������܂� -1�̎��̓f�t�H���g�ɐݒ肳��܂�
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