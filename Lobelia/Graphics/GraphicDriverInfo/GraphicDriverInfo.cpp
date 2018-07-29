#include "Common/Common.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	GraphicDriverInfo::GraphicDriverInfo(const ComPtr<IDXGIAdapter>& adapter) :adapter(adapter) {
		HRESULT hr = this->adapter->GetDesc(&desc);
		if (FAILED(hr))STRICT_THROW("ドライバー情報の取得に失敗しました");
	}
	GraphicDriverInfo::~GraphicDriverInfo() = default;
	std::string GraphicDriverInfo::GetDriverName()const noexcept { return Utility::ConverteString(desc.Description); }
	UINT GraphicDriverInfo::GetVendorId()const noexcept { return desc.VendorId; }
	UINT GraphicDriverInfo::GetDeviceId()const noexcept { return desc.DeviceId; }
	UINT GraphicDriverInfo::GetSubSysId()const noexcept { return desc.SubSysId; }
	UINT GraphicDriverInfo::GetRevision()const noexcept { return desc.Revision; }
	size_t GraphicDriverInfo::GetVideoMemory()const noexcept { return desc.DedicatedVideoMemory; }
	size_t GraphicDriverInfo::GetSystemMemory()const noexcept { return desc.DedicatedSystemMemory; }
	size_t GraphicDriverInfo::GetSharedSystemMemory()const noexcept { return desc.SharedSystemMemory; }
	LUID GraphicDriverInfo::GetLUID()const noexcept { return desc.AdapterLuid; }

	ComPtr<IDXGIFactory> GraphicDriverInfoList::factory = nullptr;
	std::vector<std::shared_ptr<GraphicDriverInfo>> GraphicDriverInfoList::drivers;
	int GraphicDriverInfoList::driverCount = 0;

	void GraphicDriverInfoList::Bootup() {
		Shutdown();
		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("ファクトリーの作成に失敗しました");
		for (driverCount = 0;; driverCount++) {
			ComPtr<IDXGIAdapter> renderTransform;
			HRESULT hr = factory->EnumAdapters(driverCount, renderTransform.GetAddressOf());
			if (hr == DXGI_ERROR_NOT_FOUND)break;
			drivers.push_back(std::make_shared<GraphicDriverInfo>(renderTransform));
		}
	}
	void GraphicDriverInfoList::Shutdown() { drivers.clear(); driverCount = 0; factory.Reset(); }
	int GraphicDriverInfoList::GetGraphicDriverCount() { return driverCount; }
	const std::shared_ptr<GraphicDriverInfo>& GraphicDriverInfoList::GetDriver(int index) { return drivers[index]; }
	const std::vector<std::shared_ptr<GraphicDriverInfo>>& GraphicDriverInfoList::GetDrivers() { return drivers; }

}