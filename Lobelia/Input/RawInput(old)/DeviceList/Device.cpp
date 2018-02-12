#include "Common/Common.hpp"
#include "Device.hpp"

namespace Lobelia::Input {
	Device::Device(HANDLE handle) :handle(handle) {}
	Device::~Device() = default;
	HANDLE Device::GetHandle() { return handle; }
}