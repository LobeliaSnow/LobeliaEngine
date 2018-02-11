#pragma once
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")

namespace Lobelia::Input {
	namespace Experimental {
		//デバイスを所持
		class DirectInput :public Utility::Singleton<DirectInput> {
			friend class Utility::Singleton<DirectInput>;
		private:
			HINSTANCE inst;
			LPDIRECTINPUT8 device;
		public:
			void Initialize();
			LPDIRECTINPUT8 GetDevice();
		private:
			DirectInput() = default;
			~DirectInput() = default;
			DirectInput(const DirectInput&) = delete;
			DirectInput(DirectInput&&) = delete;
			DirectInput& operator =(const DirectInput&) = delete;
			DirectInput& operator =(DirectInput&&) = delete;
		};
	}
	inline namespace Temp {
		class Keyboard :public Utility::Singleton<Keyboard> {
		private:
			BYTE buffer[256] = {};
		public:
			void Update() {
				for (int i = 0; i < 256; i++) {
					buffer[i] <<= 1;
					if (GetAsyncKeyState(i))buffer[i] |= 1;
				}
			}
			BYTE GetKey(int key_code) { return buffer[key_code] & 3; }
		};
		inline BYTE GetKeyboardKey(int key_code) { return Keyboard::GetInstance()->GetKey(key_code); }
		inline BYTE GetMouseKey(int key_code) { return 0; }
	}

}