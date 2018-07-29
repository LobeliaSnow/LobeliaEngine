#include "Lobelia.hpp"
#include <Windowsx.h>

namespace Lobelia::Game {
	IME::IME() {
		context = ImmGetContext(Application::GetInstance()->GetWindow()->GetHandle());
		//初期はIMEが閉じている
		Open(true);
		//入力ボックスの受け入れ可能文字数
		inputBox.reserve(1024);
		DWORD conversion;
		DWORD sentence;
		ImmGetConversionStatus(context, &conversion, &sentence);
		POINT pt = { 100,100 };
		ImmSetStatusWindowPos(context, &pt);
	}
	IME::~IME() {
		ImmReleaseContext(Application::GetInstance()->GetWindow()->GetHandle(), context);
	}
	HKL IME::GetKeyboardLocale() { return GetKeyboardLayout(0); }
	void IME::Open(bool open) { ImmSetOpenStatus(context, open); }
	bool IME::IsOpen() { return ImmGetOpenStatus(context); }
	bool IME::IsIME() { return ImmIsIME(GetKeyboardLocale()); }
	void IME::OpenConfig(ConfigMode mode, void* data) {
		HKL locale = GetKeyboardLocale();
		if (!IsIME()) STRICT_THROW("IMEが存在しません");
		ImmConfigureIME(locale, Application::GetInstance()->GetWindow()->GetHandle(), i_cast(mode), data);
	}
	void IME::Func() {
		//DWORD conversion;
		//DWORD sentence;
		//char imeName[32] = {};
		//ImmGetDescription(GetKeyboardLocale(), imeName, sizeof(imeName));
		//ImmGetConversionStatus(context, &conversion, &sentence);

	}
	void IME::Procedure(UINT msg, WPARAM wp, LPARAM lp) {
		char szBuf[1024];
		switch (msg) {
		case WM_IME_COMPOSITION:
			//変換確定時
			if (lp & GCS_RESULTSTR) {
				memset(szBuf, '\0', 1024);
				ImmGetCompositionString(context, GCS_RESULTSTR, szBuf, sizeof(szBuf));
				inputBox = szBuf;
			}
			//入力時
			if (lp & GCS_COMPSTR) {
				memset(szBuf, '\0', 1024);
				ImmGetCompositionString(context, GCS_COMPSTR, szBuf, sizeof(szBuf));
				inputBox = szBuf;
			}
			break;
		case WM_IME_NOTIFY:
			//変換候補が変わった時
			if (wp == IMN_CHANGECANDIDATE) {
				break;
			}
		}
		//ImmGetCompositionString(context,)
	}
}