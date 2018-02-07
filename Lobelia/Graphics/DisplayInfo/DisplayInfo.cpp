#include "Common/Common.hpp"
#include "Graphics/DisplayInfo/DisplayInfo.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	DisplayInfo::DisplayInfo(const ComPtr<IDXGIOutput>& output, int index) :output(output) {
		HRESULT hr = output->GetDesc(&desc);
		if (FAILED(hr))STRICT_THROW("ウインドウ情報取得に失敗しました");
		std::string displayName = Utility::ConverteString(desc.DeviceName);
		hdc = CreateDC("DISPLAY", displayName.c_str(), nullptr, nullptr);
		refreshRate = GetDeviceCaps(hdc, VREFRESH);
		driverVersion = GetDeviceCaps(hdc, DRIVERVERSION);
		size.x = static_cast<float>(GetDeviceCaps(hdc, HORZSIZE));
		size.y = static_cast<float>(GetDeviceCaps(hdc, VERTSIZE));
		device.cb = sizeof(DISPLAY_DEVICE);
		EnumDisplayDevices(nullptr, index, &device, 0);
	}
	DisplayInfo::~DisplayInfo() { DeleteDC(hdc); }
	HDC DisplayInfo::GetHDC()const noexcept { return hdc; }
	std::string DisplayInfo::GetOutputGPUName()const noexcept { return device.DeviceString; }
	std::string DisplayInfo::GetID()const noexcept { return device.DeviceID; }
	std::string DisplayInfo::GetKey()const noexcept { return device.DeviceKey; }
	int DisplayInfo::GetRefreshRate()const noexcept { return refreshRate; }
	int DisplayInfo::GetDriverVersion()const noexcept { return driverVersion; }
	RECT DisplayInfo::GetDesktopRect()const noexcept { return desc.DesktopCoordinates; }
	int DisplayInfo::GetAttachedDesktopCount()const noexcept { return desc.AttachedToDesktop; }
	DXGI_MODE_ROTATION DisplayInfo::GetRotationMode()const noexcept { return desc.Rotation; }
	Math::Vector2 DisplayInfo::GetPhysicSize()const noexcept { return size; }

	ComPtr<IDXGIFactory> DisplayInfoList::factory = nullptr;
	std::vector<std::shared_ptr<DisplayInfo>> DisplayInfoList::displays;
	int DisplayInfoList::displayCount = 0;
	void DisplayInfoList::Bootup() {
		Shutdown();
		HRESULT hr = S_OK;
		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("ファクトリーの作成に失敗しました");
		ComPtr<IDXGIAdapter> adapter;
		hr = factory->EnumAdapters(0, adapter.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("アダプターの列挙に失敗");
		for (displayCount = 0;; displayCount++) {
			ComPtr<IDXGIOutput> output;
			HRESULT hr = adapter->EnumOutputs(displayCount, output.GetAddressOf());
			if (hr == DXGI_ERROR_NOT_FOUND)break;
			displays.push_back(std::make_shared<DisplayInfo>(output, displayCount));
		}
	}
	void DisplayInfoList::Shutdown() { factory.Reset(); displays.clear(); displayCount = 0; }
	int DisplayInfoList::GetDisplayCount() { return displayCount; }
	const std::shared_ptr<DisplayInfo>& DisplayInfoList::GetDisplayInfo(int index) { return displays[index]; }
	const std::vector<std::shared_ptr<DisplayInfo>>& DisplayInfoList::GetDisplaysInfo() { return displays; }

}