#pragma once
namespace Lobelia::Utility {
	template<class T> class Singleton {
	protected:
		__forceinline Singleton()noexcept = default;
		__forceinline virtual ~Singleton()noexcept = default;
	public:
		static T* GetInstance() {
			static T inst;
			return &inst;
		}
	};
}
