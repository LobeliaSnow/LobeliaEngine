#pragma once
namespace Lobelia::Input {
	class Mouse :public Device {
		friend class DeviceManager;
	private:
		MouseData data;
	private:
		void Update(const MouseData& data);
	public:
		Mouse(HANDLE handle);
		~Mouse();
		BYTE GetKey(int key_code);
		Math::Vector2 GetMove();
		int GetWheel();
	};
	class WindowsMousePointer :public Utility::Singleton<WindowsMousePointer>{
		friend class Utility::Singleton<WindowsMousePointer>;
	private:
		Math::Vector2 pos;
		WindowsMousePointer() = default;
		~WindowsMousePointer() = default;
	public:
		void Update();
		Math::Vector2 GetPos() { return pos; }
	};
}