#pragma once
#include <dwrite.h>
#include <d2d1_1.h>
#pragma comment(lib,"Dwrite.lib")
#pragma comment(lib,"D2D1.lib")

namespace Lobelia::Graphics {
	class Direct2DSystem {
	private:
		/**@brief D2D�f�o�C�X*/
		static ComPtr<ID2D1Device> device;
		/**@brief D2D�f�o�C�X�R���e�L�X�g*/
		static ComPtr<ID2D1DeviceContext> context;
		static Math::Vector2 dpi;
	public:
		static bool Initialize();
		/**@brief D2D�f�o�C�X�R���e�L�X�g�擾*/
		static ComPtr<ID2D1DeviceContext> GetContext();
		static Math::Vector2 GetDpi();
	};

	class Direct2DRenderTarget {
	private:
		/**@brief D2D�p�r�b�g�}�b�v*/
		ComPtr<ID2D1Bitmap1> bitmap;
	public:
		Direct2DRenderTarget(SwapChain* swap_chain);
		~Direct2DRenderTarget();
		const ComPtr<ID2D1Bitmap1>& Get();
		void Set();
	};

	class Font {
		friend class Direct2DRenderer;
	private:
		/**@brief �t�H���g*/
		ComPtr<IDWriteTextFormat>	font;
	public:
		/**
		*@brief �R���X�g���N�^
		*@param[in] �t�H���g��
		*@param[in] �����T�C�Y
		*/
		Font(const char* font_name, UINT size);
		virtual ~Font();
	};

	class Direct2DRenderer final {
	private:
		static std::unique_ptr<Direct2DRenderTarget> renderTarget;
		static ComPtr<ID2D1SolidColorBrush> brush;
	public:
		static void Initialize();
		static void BeginRender();
		//�F�ύX���g�p �O�̃u���V�͔j��
		static void ColorChange(Utility::Color color = 0xFFFFFFFF);
		static void CircleRender(const Math::Vector2& center, const Math::Vector2& radius, float stroke_width = 1.0f);
		static void LineRender(const Math::Vector2& p1, const Math::Vector2& p2, float stroke_width = 1.0f);
		static void SquareRender(const RECT& rect, float stroke_width = 1.0f);
		static void SquareRender(const Math::Vector2& pos, const Math::Vector2& size, float stroke_width = 1.0f);
		static void FontRender(Font* font, const Math::Vector2& pos, const char* str);
		template<class... Args> static void FontRender(Font* font, const Math::Vector2& pos, const char* format, Args... args) {
			char buffer[256] = {};
			sprintf_s(buffer, format, args...);
			FontRender(font, pos, buffer);
		}
		static void EndRender();
	};


}