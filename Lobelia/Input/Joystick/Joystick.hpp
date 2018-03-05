#pragma once
#ifdef USE_XINPUT
#include <Xinput.h>
//#pragma comment (lib, "xinput.lib")
#pragma comment(lib,"Xinput9_1_0.lib")
#endif

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
			//�̃t���[�������ꂽ�u�Ԃ�ID
			int pushKey;
			std::string deviceName;
			int deadZone;
			Controller(bool is_xinput, const char* device_name);
		};
		class DirectInputController :public InputDevice {
		private:
			static BOOL CALLBACK EnumAxis(LPCDIDEVICEOBJECTINSTANCE inst, LPVOID ref);
		private:
			int asignMap[16];
			int pov[4];
			std::string deviceName;
			ComPtr<IDirectInputEffect> effect;
		private:
			bool Poll();
		private:
			void CreateEffect();
		public:
			DirectInputController(HWND hwnd, const GUID& guid, const DIDATAFORMAT& format, bool fore_ground, bool exclusive);
			~DirectInputController() = default;
			void SetPadSet(const DirectInputPadSet& padset);
			void Update(Controller* data);
			//�t�H�[�X�t�B�[�h�o�b�N�Ή��R���g���[���[�̂�
			void Vibration(int gain, float period);
			const std::string& GetDeviceName();
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
#ifdef USE_XINPUT
		void XInputUpdate();
#endif
		void CheckIndex(int index);
	public:
		void Initialize(HWND hwnd, bool fore_ground = true, bool exclusive = false);
		//XInput�̎��͖�������܂� -1�̎��̓f�t�H���g�ɐݒ肳��܂�
		void SetDirectInputPadSet(int index, const DirectInputPadSet& padset);
		void Update();
		int GetControllerCount();
		BYTE GetKey(int index, KeyCode code);
		bool IsPushAnyKey(int index);
		int PushKeyNo(int index);
		const Math::Vector2& GetLeftAxis(int index);
		const Math::Vector2& GetRightAxis(int index);
		const std::string& GetDeviceName(int index);
		//power�͈̔͂�0~65535�܂łł�
		void Vibration(int index, float power);
		//0~1000�܂�
		void SetDeadZone(int index, int dead_zone);
	private:
		Joystick() = default;
		~Joystick() = default;
		Joystick(const Joystick&) = delete;
		Joystick(Joystick&&) = delete;
		Joystick& operator =(const Joystick&) = delete;
		Joystick& operator =(Joystick&&) = delete;
	};
}