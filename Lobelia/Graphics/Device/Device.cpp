#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"

namespace Lobelia::Graphics {
	ComPtr<ID3D11Device> Device::device;
	ComPtr<ID3D11DeviceContext> Device::context;

	void Device::Create(UINT device_flag, GraphicDriverInfo* info) {
		Destroy();
		HRESULT hr = S_OK;
		//これを外から決めれるようにするか否か。。。
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,D3D_FEATURE_LEVEL_9_2,D3D_FEATURE_LEVEL_9_1 };
		IDXGIAdapter* adapter = nullptr;
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE;
		if (info) {
			adapter = info->adapter.Get();
			driverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN;
		}
		hr = D3D11CreateDevice(adapter, driverType, nullptr, device_flag, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, context.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("デバイスの作成に失敗しました");
	}
	void Device::Destroy() {
		device.Reset();
	}

	ComPtr<ID3D11Device>& Device::Get() { return device; }
	ComPtr<ID3D11DeviceContext>& Device::GetContext() { return context; }


}