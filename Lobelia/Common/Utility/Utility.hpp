#pragma once

namespace Lobelia {
	using Microsoft::WRL::ComPtr;
}
namespace Lobelia::Utility {
	inline std::wstring ConverteWString(const std::string& str)noexcept {
		wchar_t name[256] = {};
		size_t strSum = {};
		//setlocale(LC_CTYPE, "jpn");
		errno_t err = mbstowcs_s(&strSum, name, 256, str.c_str(), 256);
		std::wstring wstr = name;
		return wstr;
	}
	inline std::string ConverteString(const std::wstring& str)noexcept {
		char name[256] = {};
		size_t strSum = {};
		//setlocale(LC_CTYPE, "jpn");
		errno_t err = wcstombs_s(&strSum, name, 256, str.c_str(), 256);
		std::string wstr = name;
		return wstr;
	}
	template<class First, class ...Arg> inline int Sum(const First& first, Arg ...arg) {
		return first + Sum(arg...);
	}
	template<class T, class ...Arg> inline void SafeNew(T** p, Arg ...arg) {
		SafeDelete(*p);
		*p = new T(arg...);
	}
	template<class T> inline void SafeDelete(T* p) {
		if (p != nullptr) {
			delete p;
			p = nullptr;
		}
	}

	inline float Frand(float min, float max) { return (static_cast<float>(rand()) / RAND_MAX)*(max - min) + min; }
}
#include "Color.hpp"
#include "Singleton.hpp"
#include "ResourceBank/ResourceBank.hpp"
#include "FilePathControl/FilePathControl.hpp"
#include "FileController/FileController.hpp"
