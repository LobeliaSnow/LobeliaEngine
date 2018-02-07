#include "Common/Common.hpp"
#include "Input/DeviceList/Device.hpp"
#include "Input/Data/Data.hpp"
#include "DS4.hpp"
namespace Lobelia::Input {
	DualShock4::DualShock4(HANDLE device) :device(device) {

	}
	void DualShock4::Update(const DualShock4Data& data) {
		_::Push(this->data.buffer.data(), data.buffer.data(), 14);
	}
	HANDLE DualShock4::GetHandle() { return device; }
	BYTE DualShock4::GetKey(KeyCode key_code) { return data.buffer[key_code] & 3; }
	bool DualShock4::IsDualShock4(const RID_DEVICE_INFO& info) {
		return (info.hid.dwVendorId == 0x054C);
	}

}