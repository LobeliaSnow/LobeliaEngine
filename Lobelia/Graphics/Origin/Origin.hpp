#pragma once
namespace Lobelia::Graphics {
	template<class T> class Origin {
	private:
		int uniqueId;
	public:
		Origin();
		~Origin();
		bool IsSet();
	};
}
#include "Origin.inl"
