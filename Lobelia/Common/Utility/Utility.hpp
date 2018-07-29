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
	//CreateContainer�̃w���p�[�֐��Q
	namespace _ {
		//����^�̏ꍇ
		template<template<class, class> class Container, class Type> void PushBack(Container<Type, std::allocator<Type>>& container, const Type& val) {
			//�x���΍�
			container.push_back(s_cast<Type>(val));
		}
		//�z��̏ꍇ
		template<template<class, class> class Container, class Type, size_t _size> void PushBack(Container<Type, std::allocator<Type>>& container, const Type(&val)[_size]) {
			for (int i = 0; i < _size; i++) { PushBack(container, val[i]); }
		}
		//���̑�(�R���e�i�^)�̏ꍇ
		//begin end�����邩�m���߂Ă��ق����x�^�[����
		template<class Container, class Type> void PushBack(Container& container, const Type& val) {
			for each(auto&& it in val) { PushBack(container, it); }
		}
	}
	//�R���e�i���쐬���邽�߂̊֐�
	//�ϐ����z����R���e�i���������ɓ�����Ƃ��̏��Ԓʂ�ɑS����̃R���e�i�ɘA�����ďo�Ă��܂�
	//vector�̏ꍇ��
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