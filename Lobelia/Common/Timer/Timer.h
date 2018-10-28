#pragma once
namespace Lobelia {
	class Timer {
	public:
		Timer() noexcept;
		__forceinline ~Timer()noexcept;
		__forceinline void Begin()noexcept;
		__forceinline void End()noexcept;
		__forceinline float GetMilisecondResult()const noexcept;
		__forceinline float GetSecondResult()const noexcept;
	private:
		LARGE_INTEGER freq;
		/**@brief �t���[���J�n����*/
		LARGE_INTEGER firstTime;
		/**@brief �t���[���I������*/
		LARGE_INTEGER endTime;
	};
}
#include "Timer.inl"