#pragma once
namespace Lobelia::Input {
	template<class T> inline BYTE DeviceManager::GetKey(int key_code) { static_assert(1, "Type Error!"); }
	template<> inline BYTE DeviceManager::GetKey<Keyboard>(int key_code) {
		int keyCount = GetKeyboardCount();
		for (int i = 0; i < keyCount; i++) {
			BYTE ret = keyboards[i].second->GetKey(key_code);
			if (ret != 0)return ret;
		}
		return 0;
	}
	template<> inline BYTE DeviceManager::GetKey<Mouse>(int key_code) {
		int mouseCount = GetMouseCount();
		for (int i = 0; i < mouseCount; i++) {
			BYTE ret = mouses[i].second->GetKey(key_code);
			if (ret != 0)return ret;
		}
		return 0;
	}
	template<> inline BYTE DeviceManager::GetKey<DualShock4>(int key_code) {
		int keyCount = GetDualShock4Count();
		for (int i = 0; i < keyCount; i++) {
			BYTE ret = dualShock4s[i].second->GetKey(s_cast<DualShock4::KeyCode>(key_code));
			if (ret != 0)return ret;
		}
		return 0;
	}

}