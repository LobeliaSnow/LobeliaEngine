namespace Lobelia {
	__forceinline Timer::Timer() noexcept {
		QueryPerformanceFrequency(&freq);
	}
	__forceinline Timer::~Timer()noexcept = default;
	__forceinline void Timer::Begin()noexcept { QueryPerformanceCounter(&firstTime); }
	__forceinline void Timer::End()noexcept { QueryPerformanceCounter(&endTime); }
	__forceinline float Timer::GetMilisecondResult()const noexcept {
		float unit = 1000.0f;	unit /= freq.QuadPart;
		return static_cast<float>((endTime.QuadPart - firstTime.QuadPart) * unit);
	}
	__forceinline float Timer::GetSecondResult()const noexcept { return GetMilisecondResult() / 1000.0f; }

}