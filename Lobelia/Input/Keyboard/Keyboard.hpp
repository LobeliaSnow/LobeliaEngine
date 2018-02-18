#pragma once
namespace Lobelia::Input {
	class Keyboard :protected InputDevice, public Utility::Singleton<Keyboard> {
		friend class Utility::Singleton<Keyboard>;
	private:
		BYTE buffer[256];
		int pushKeyNo;
	public:
		void Initialize(HWND hwnd, bool fore_ground = true, bool exclusive = false);
		void Update();
		BYTE GetKey(int key_code);
		bool IsPushAnyKey();
		int PushKeyNo();
	private:
		Keyboard();
		~Keyboard() = default;
		Keyboard(const Keyboard&) = delete;
		Keyboard(Keyboard&&) = delete;
		Keyboard& operator =(const Keyboard&) = delete;
		Keyboard& operator =(Keyboard&&) = delete;
	};
}