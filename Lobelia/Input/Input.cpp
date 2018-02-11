#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Input.hpp"

namespace Lobelia::Input {
	namespace Experimental {
		void DirectInput::Initialize() {
			HRESULT hr = S_OK;
			hr = DirectInput8Create(inst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_lpDI, NULL);
			if (FAILED(hr))STRICT_THROW("DirectInput�f�o�C�X�̍쐬�Ɏ��s");
		}
		LPDIRECTINPUT8 DirectInput::GetDevice() { return device; }
	}
}