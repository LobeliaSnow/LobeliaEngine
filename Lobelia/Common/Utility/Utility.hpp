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
	template<class First, class ...Arg> inline float Sum(const First& first, Arg ...arg) {
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
	//CreateContainerのヘルパー関数群
	namespace _ {
		//同一型の場合
		template<template<class, class> class Container, class Type> void PushBack(Container<Type, std::allocator<Type>>& container, const Type& val) {
			//警告対策
			container.push_back(s_cast<Type>(val));
		}
		//配列の場合
		template<template<class, class> class Container, class Type, size_t _size> void PushBack(Container<Type, std::allocator<Type>>& container, const Type(&val)[_size]) {
			for (int i = 0; i < _size; i++) { PushBack(container, val[i]); }
		}
		//その他(コンテナ型)の場合
		//begin endがあるか確かめてやるほうがベターかも
		template<class Container, class Type> void PushBack(Container& container, const Type& val) {
			for each(auto&& it in val) { PushBack(container, it); }
		}
	}
	//コンテナを作成するための関数
	//変数やら配列やらコンテナやらを引数に投げるとその順番通りに全部一つのコンテナに連結して出てきます
	//vectorの場合は
	template<template<class, class> class Container, class Type, class... Args> void CreateContainer(Container<Type, std::allocator<Type>>& container, Args&&... args) {
		(void)std::initializer_list<int> {
			(void(_::PushBack(container, args)), 0)...
		};
	}

}
#include "Color.hpp"
#include "Singleton.hpp"
#include "ResourceBank/ResourceBank.hpp"
#include "FilePathControl/FilePathControl.hpp"
#include "FileController/FileController.hpp"
#include "PerformanceCounter/PerformanceCounter.hpp"
#include "MultiThread/MultiThread.hpp"