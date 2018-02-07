#pragma once
namespace Lobelia {
	class Window {
	private:
		HWND hwnd;
		Math::Vector2 size;
		const std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)> WND_PROC;
	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
		__forceinline bool MakeWindow(Math::Vector2 size, const char* w_name, DWORD style, HWND parent);
	public:
		Window(const Math::Vector2& size, const char* w_name = APPLICATION_NAME, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)> wnd_proc = DefWindowProc, DWORD style = 0, HWND parent = nullptr);
		virtual ~Window();
		const HWND& GetHandle()const noexcept;
		void ShowWindow(int n_cmd_show)noexcept;
		void UppdateWindow()noexcept;
		const Math::Vector2& GetSize()const noexcept;
		void DetectionSize()noexcept;
	};
}

