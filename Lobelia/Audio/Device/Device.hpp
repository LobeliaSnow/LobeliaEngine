#pragma once
#include "XAudio2.h"
#include "XAudio2fx.h"
#include <x3daudio.h>
#pragma comment(lib,"X3DAudio.lib")

namespace Lobelia::Audio {
	using Microsoft::WRL::ComPtr;
	class Device {
	private:
		static ComPtr<IXAudio2> device;
		static XAUDIO2_DEVICE_DETAILS details;
	public:
		static void Create();
		static void Destroy();
		static IXAudio2* Get();
		static const XAUDIO2_DEVICE_DETAILS& GetDetails();
		//midi‚Ìƒm[ƒg”Ô†‚ğˆø”‚Å“n‚·‚ÆA‚»‚Ìü”g”‚ª•Ô‚Á‚Ä‚«‚Ü‚·
		static float CalcMIDIHz(int note);

	};
}