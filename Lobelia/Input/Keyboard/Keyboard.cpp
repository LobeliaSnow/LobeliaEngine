#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Input/Input.hpp"
#include "Keyboard.hpp"

namespace Lobelia::Input {
	namespace Experimental {
		void Keyboard::Initialize() {
			auto device = DirectInput::GetInstance()->GetDevice();
			//device->CreateDevice
		}
	}
}
