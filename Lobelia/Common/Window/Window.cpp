#include <Windows.h>
#include <functional>
#include "Common/Define/Define.hpp"
#include "Math/Math.hpp"
#include "Window.h"
namespace Lobelia {
	LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (window)	return window->WND_PROC(hwnd, msg, wp, lp);
		//取得できなかった場合
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	bool Window::MakeWindow(Math::Vector2 size, const char* w_name, DWORD style, HWND parent) {
		WNDCLASSEX wc = {};
		HINSTANCE hinst = GetModuleHandle(nullptr);
		wc.cbSize = sizeof WNDCLASSEX;
		wc.hInstance = hinst;
		wc.lpfnWndProc = WndProc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		//アイコン設定 ここでアイコンを変えることもできる
		wc.hIcon = LoadIcon(hinst, "ICONPIC");
		//wc.hIcon = LoadIcon(NULL, IDC_ICON);
		//マウスカーソル設定 ここでカーソルを変えることもできる
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		//wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
		wc.hbrBackground = NULL;
		wc.lpszClassName = APPLICATION_NAME;
		wc.hIconSm = nullptr;
		RegisterClassEx(&wc);
		RECT rc = { 0, 0, size.Get().x, size.Get().y };
		AdjustWindowRectEx(&rc, style, false, 0);
		//ウインドウ作成
		hwnd = CreateWindowEx(0, APPLICATION_NAME, w_name, style, 0, 0, rc.right - rc.left, rc.bottom - rc.top, parent, nullptr, hinst, nullptr);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		return true;
	}
	Window::Window(const Math::Vector2& size, const char* w_name, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)>wnd_proc, DWORD style, HWND parent) :hwnd(nullptr), size(size), WND_PROC(wnd_proc) {
		MakeWindow(size, w_name, style, parent);
	}
	Window::~Window() = default;
	const HWND& Window::GetHandle() const noexcept { return hwnd; }
	void Window::ShowWindow(int n_cmd_show) noexcept { ::ShowWindow(hwnd, n_cmd_show); }
	void Window::UppdateWindow() noexcept { ::UpdateWindow(hwnd); }
	const Math::Vector2& Window::GetSize()const noexcept { return size; }
	void Window::DetectionSize()noexcept {
		RECT rc = {};
		GetClientRect(hwnd, &rc);
		size.y = f_cast(rc.bottom - rc.top);
		size.x = f_cast(rc.right - rc.left);
	}
}