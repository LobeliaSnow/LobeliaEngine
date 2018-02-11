#include "Common/Common.hpp"
#include "Device.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Audio {
	ComPtr<IXAudio2> Device::device;
	XAUDIO2_DEVICE_DETAILS Device::details = {};

	void Device::Create() {
		UINT32 flags = 0;
#ifdef _DEBUG
		flags |= XAUDIO2_DEBUG_ENGINE;
#endif
		HRESULT hr = S_OK;
		hr = XAudio2Create(&device, flags);
		if (FAILED(hr))STRICT_THROW("XAudioEngineの初期化に失敗");
		//デバイス情報を取得(今回はプライマリのみ取得、もし抜かれてしまった場合の処理も考えないといけない？)
		hr = Device::Get()->GetDeviceDetails(0, &details);
		if (FAILED(hr))STRICT_THROW("");
	}
	void Device::Destroy() { device.Reset(); }
	IXAudio2* Device::Get() { return device.Get(); }
	const XAUDIO2_DEVICE_DETAILS& Device::GetDetails() { return details; }
	float Device::CalcMIDIHz(int note) {
		return 440.0f * f_cast(pow(2.0, (s_cast<double>(note) - 69.0) / 12.0f));
	}

}