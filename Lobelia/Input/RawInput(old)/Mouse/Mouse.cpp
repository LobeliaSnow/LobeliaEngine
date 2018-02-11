#include "Lobelia.hpp"

namespace Lobelia::Input {
	Mouse::Mouse(HANDLE handle) :Device(handle), data{} {}
	Mouse::~Mouse() = default;
	void Mouse::Update(const MouseData& data) {
		_::Push(this->data.buffer.data(), data.buffer.data(), 3);
		this->data.move = data.move;
		this->data.wheel = data.wheel;
	}
	BYTE Mouse::GetKey(int key_code) { return data.buffer[key_code] & 3; }
	Math::Vector2 Mouse::GetMove() { return data.move; }
	int Mouse::GetWheel() { return data.wheel; }

	void WindowsMousePointer::Update() {
		POINT mpos = {};
		GetCursorPos(&mpos);
		ScreenToClient(Application::GetInstance()->GetWindow()->GetHandle(), &mpos);
		pos.x = f_cast(mpos.x);	pos.y = f_cast(mpos.y);
	}
}