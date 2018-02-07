#pragma once
namespace Lobelia::Graphics {
	class ShaderBank {
	public:
		template<class ShaderElement, class ...Args> static void Register(const std::string& key, Args ...args);
		template<class ShaderElement> static void UnRegister(const std::string& key);
		template<class ShaderElement> static ShaderElement* Get(const std::string& key);
	};
}

#include "ShaderBank.inl"