#include "Common/Common.hpp"
#include "Input/DeviceList/Device.hpp"
#include "Input/Data/Data.hpp"
#include "../Mouse/Mouse.hpp"
#include "Keyboard.hpp"


namespace Lobelia::Input {
	Keyboard::Keyboard(HANDLE handle) :Device(handle) {}
	Keyboard::~Keyboard() = default;

	void Keyboard::Update(const KeyboardData& data) {
		_::Push(this->data.buffer.data(), data.buffer.data(), 256);
	}
	int Keyboard::GetKey(int key_code) { return data.buffer[key_code] & 3; }
}