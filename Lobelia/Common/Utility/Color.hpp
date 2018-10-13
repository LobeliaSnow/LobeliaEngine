#pragma once
namespace Lobelia::Utility {
	struct Color {
	public:
		union {
			DWORD colorCode;
			struct { BYTE bgra[4]; };
			struct { BYTE b, g, r, a; };
		};
		__forceinline constexpr Color(DWORD cCode = 0xFFFFFFFF)noexcept :colorCode(cCode) {}
		__forceinline Color(BYTE b, BYTE g, BYTE r, BYTE a) : b(b), g(g), r(r), a(a) {}
		__forceinline ~Color()noexcept = default;
		__forceinline constexpr operator DWORD()const noexcept { return colorCode; }
		__forceinline operator Math::Vector4()const noexcept { return Math::Vector4(GetNormalizedR(), GetNormalizedG(), GetNormalizedB(), GetNormalizedA()); }
		__forceinline void operator =(DWORD c)noexcept { colorCode = c; }
		__forceinline float GetNormalizedA()const noexcept { return static_cast<float>(a) / 255.0f; }
		__forceinline float GetNormalizedR()const noexcept { return static_cast<float>(r) / 255.0f; }
		__forceinline float GetNormalizedG()const noexcept { return static_cast<float>(g) / 255.0f; }
		__forceinline float GetNormalizedB()const noexcept { return static_cast<float>(b) / 255.0f; }
	};
}