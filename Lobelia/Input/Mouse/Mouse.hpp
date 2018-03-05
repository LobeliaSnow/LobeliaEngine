#pragma once
namespace Lobelia::Input {
	class Mouse :protected InputDevice, public Utility::Singleton<Mouse> {
		friend class Utility::Singleton<Mouse>;
	private:
		BYTE buffer[3];
		Math::Vector2 move;
		int wheel;
		Math::Vector2 clientPos;
		Math::Vector2 screenPos;
		int pushKeyNo;
	public:
		void Initialize(HWND hwnd, bool fore_ground = true, bool exclusive = false);
		void Update();
		BYTE GetKey(int key_code);
		bool IsPushAnyKey();
		int PushKeyNo();
		const Math::Vector2& GetMove();
		int GetWheel();
		const Math::Vector2& GetClientPos();
		const Math::Vector2& GetScreenPos();
	private:
		Mouse();
		~Mouse() = default;
		Mouse(const Mouse&) = delete;
		Mouse(Mouse&&) = delete;
		Mouse& operator =(const Mouse&) = delete;
		Mouse& operator =(Mouse&&) = delete;
	};
}