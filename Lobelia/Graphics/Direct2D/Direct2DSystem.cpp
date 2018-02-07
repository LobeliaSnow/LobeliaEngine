#include "Lobelia.hpp"
namespace Lobelia::Graphics {
	ComPtr<ID2D1Device> Direct2DSystem::device;
	ComPtr<ID2D1DeviceContext> Direct2DSystem::context;
	Math::Vector2 Direct2DSystem::dpi;

	bool Direct2DSystem::Initialize() {
		HRESULT hr = S_OK;
		//ファクトリー作成
		D2D1_FACTORY_OPTIONS d2dOpt;
		ZeroMemory(&d2dOpt, sizeof d2dOpt);
		ComPtr<ID2D1Factory1> d2dFactory;
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(d2dFactory.GetAddressOf()));
		if (hr == E_NOINTERFACE)STRICT_THROW("アップデートが必要です");
		if (FAILED(hr))STRICT_THROW("ファクトリー作成に失敗");
		//DPIの取得
		d2dFactory->GetDesktopDpi(&dpi.x, &dpi.y);
		ComPtr<IDXGIDevice1> dxgiDevice;
		hr = Device::Get().Get()->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(dxgiDevice.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("DXGIデバイス作成に失敗");
		//Direct2Dデバイス作成
		hr = d2dFactory->CreateDevice(dxgiDevice.Get(), &device);
		if (FAILED(hr))STRICT_THROW("D2Dデバイス作成に失敗");
		//Direct2Dデバイスコンテキストの作成
		hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);
		if (FAILED(hr))STRICT_THROW("デバイスコンテキストの作成に失敗");
		return true;
	}
	ComPtr<ID2D1DeviceContext> Direct2DSystem::GetContext() { return context; }
	Math::Vector2 Direct2DSystem::GetDpi() { return dpi; }

	Direct2DRenderTarget::Direct2DRenderTarget(SwapChain* swap_chain) {
		ComPtr<IDXGISurface> surface;
		HRESULT hr = swap_chain->Get()->GetBuffer(0, IID_PPV_ARGS(&surface));
		//Direct2Dの描画先となるビットマップ作成
		D2D1_BITMAP_PROPERTIES1 d2dProp = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), Direct2DSystem::GetDpi().x, Direct2DSystem::GetDpi().y);
		hr = Direct2DSystem::GetContext()->CreateBitmapFromDxgiSurface(surface.Get(), &d2dProp, &bitmap);
		if (FAILED(hr))STRICT_THROW("ビットマップ作成に失敗");
	}
	Direct2DRenderTarget::~Direct2DRenderTarget() = default;
	const ComPtr<ID2D1Bitmap1>& Direct2DRenderTarget::Get() { return bitmap; }
	void Direct2DRenderTarget::Set() { Direct2DSystem::GetContext()->SetTarget(bitmap.Get()); }

	Font::Font(const char* font_name, UINT size) {
		HRESULT hr;
		ComPtr<IDWriteFactory> dWriteFactory;
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dWriteFactory), reinterpret_cast<IUnknown**>(dWriteFactory.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("DirectWriteのファクトリ作成に失敗");

		WCHAR name[50] = {};
		size_t wLen = 0;
		errno_t err = 0;
		//ロケール指定
		setlocale(LC_ALL, "japanese");
		//変換
		err = mbstowcs_s(&wLen, name, 50, font_name, _TRUNCATE);

		hr = dWriteFactory->CreateTextFormat(name, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, static_cast<FLOAT>(size), L"", &font);
		if (FAILED(hr))STRICT_THROW("フォント作成に失敗");
		hr = font->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		if (FAILED(hr))STRICT_THROW("文字位置設定に失敗");
		hr = font->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		if (FAILED(hr))STRICT_THROW("パラグラフの指定に失敗");
	}
	Font::~Font() = default;

	std::unique_ptr<Direct2DRenderTarget> Direct2DRenderer::renderTarget;
	ComPtr<ID2D1SolidColorBrush> Direct2DRenderer::brush;
	void Direct2DRenderer::Initialize() {
		renderTarget = std::make_unique<Direct2DRenderTarget>(Application::GetInstance()->GetSwapChain());
		renderTarget->Set();
		ColorChange();
	}
	void Direct2DRenderer::BeginRender() { Direct2DSystem::GetContext()->BeginDraw(); }
	void Direct2DRenderer::ColorChange(Utility::Color color) { Direct2DSystem::GetContext()->CreateSolidColorBrush(D2D1::ColorF(color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB(), color.GetNormalizedA()), &brush); }
	void Direct2DRenderer::CircleRender(const Math::Vector2& center, const Math::Vector2&  radius, float stroke_width) {
		D2D1_ELLIPSE ellipse1 = D2D1::Ellipse(D2D1::Point2F(static_cast<float>(center.x), static_cast<float>(center.y)), static_cast<float>(radius.x), static_cast<float>(radius.y));
		Direct2DSystem::GetContext()->DrawEllipse(ellipse1, brush.Get(), stroke_width);
	}
	void Direct2DRenderer::LineRender(const Math::Vector2&  p1, const Math::Vector2&  p2, float stroke_width) {
		D2D1_POINT_2F dp1{ static_cast<float>(p1.x),static_cast<float>(p1.y) }, dp2{ static_cast<float>(p2.x),static_cast<float>(p2.y) };
		Direct2DSystem::GetContext()->DrawLine(dp1, dp2, brush.Get(), stroke_width);
	}
	void Direct2DRenderer::SquareRender(const RECT& rect, float stroke_width) {
		D2D1_RECT_F drect = {};
		drect.bottom = static_cast<float>(rect.bottom); drect.top = static_cast<float>(rect.top);
		drect.left = static_cast<float>(rect.left); drect.right = static_cast<float>(rect.right);
		Direct2DSystem::GetContext()->DrawRectangle(drect, brush.Get(), stroke_width);
	}
	void Direct2DRenderer::SquareRender(const Math::Vector2&  pos, const Math::Vector2&  size, float stroke_width) {
		D2D1_RECT_F drect = {};
		drect.bottom = static_cast<float>(pos.y + size.y / 2.0f); drect.top = static_cast<float>(pos.y - size.y / 2.0f);
		drect.left = static_cast<float>(pos.x - size.x / 2.0f); drect.right = static_cast<float>(pos.x + size.x / 2.0f);
		Direct2DSystem::GetContext()->DrawRectangle(drect, brush.Get(), stroke_width);
	}
	void Direct2DRenderer::FontRender(Font* font, const Math::Vector2& pos, const char* str) {
		HRESULT hr = S_OK;

		D2D1_RECT_F rect;
		Math::Vector2 wsize = Application::GetInstance()->GetWindow()->GetSize();
		rect = D2D1::RectF(pos.x, pos.y, static_cast<float>(wsize.x), static_cast<float>(wsize.y));

		WCHAR drawText[256] = {};
		size_t wLen = 0;
		errno_t err = 0;
		//ロケール指定
		setlocale(LC_ALL, "japanese");
		//変換
		err = mbstowcs_s(&wLen, drawText, 256, str, _TRUNCATE);
		Direct2DSystem::GetContext()->DrawTextA(drawText, static_cast<UINT32>(wcslen(drawText)), font->font.Get(), &rect, brush.Get(), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE::DWRITE_MEASURING_MODE_NATURAL);
	}
	void Direct2DRenderer::EndRender() { Direct2DSystem::GetContext()->EndDraw(); }
}