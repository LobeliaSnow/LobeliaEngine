#pragma once
namespace Lobelia::Input {
	namespace Experimental {
		class Keyboard :public Utility::Singleton<Keyboard> {
		private:
			LPDIRECTINPUTDEVICE8 keyboard;
		public:

		};
	}
}