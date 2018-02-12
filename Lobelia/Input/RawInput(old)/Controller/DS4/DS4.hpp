#pragma once
#define BIT7 128
#define BIT6 64
#define BIT5 32
#define BIT4 16
#define BIT3 8
#define BIT2 4
#define BIT1 2
#define BIT0 1
//develop
#define TRIANGLE_CHECK(buffer)					(buffer[5] & BIT7)
#define CIRCLE_CHECK(buffer)						(buffer[5] & BIT6)
#define CROSS_CHECK(buffer)						(buffer[5] & BIT5)
#define SQUARE_CHECK(buffer)						(buffer[5] & BIT4)
#define R3_CHECK(buffer)								(buffer[6] & BIT7)
#define L3_CHECK(buffer)								(buffer[6] & BIT6)
#define OPTION_CHECK(buffer)						(buffer[6] & BIT5)
#define SHARE_CHECK(buffer)						(buffer[6] & BIT4)
#define R1_CHECK(buffer)								(buffer[6] & BIT1)
#define L1_CHECK(buffer)								(buffer[6] & BIT0)
#define AXIS_UP_CHECK(buffer)						((buffer[5] & 15) == 0)
#define AXIS_UPRIGHT_CHECK(buffer)			((buffer[5] & 15) == 1)
#define AXIS_RIGHT_CHECK(buffer)				((buffer[5] & 15) == 2)
#define AXIS_RIGHTDOWN_CHECK(buffer)		((buffer[5] & 15) == 3)
#define AXIS_DOWN_CHECK(buffer)				((buffer[5] & 15) == 4)
#define AXIS_DOWNLEFT_CHECK(buffer)		((buffer[5] & 15) == 5)
#define AXIS_LEFT_CHECK(buffer)					((buffer[5] & 15) == 6)
#define AXIS_LEFTUP_CHECK(buffer)				((buffer[5] & 15) == 7)
#define AXIS_NOBUTTON_CHECK(buffer)		((buffer[5] & 15) == 8)

namespace Lobelia::Input {
	class DualShock4 {
	public:
		enum KeyCode { CIRCLE, CROSS, SQUARE, TRIANGLE, UP, DOWN, LEFT, RIGHT, L1, L3, R1, R3, SHARE, OPTION, MAX };
	private:
		HANDLE device;
		DualShock4Data data;
	public:
		DualShock4(HANDLE device);
		void Update(const DualShock4Data& data);
		HANDLE GetHandle();
		BYTE GetKey(KeyCode key_code);
	public:
		static bool IsDualShock4(const RID_DEVICE_INFO& info);
	};
}