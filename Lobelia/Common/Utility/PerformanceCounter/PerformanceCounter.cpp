#include "Common/Common.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Utility {
	CPUPerformanceCounter::CPUPerformanceCounter() {
		PdhOpenQuery(nullptr, 0, &hQuery);
		// 現在のCPU％を取得
		PdhAddCounter(hQuery, "\\Processor(_Total)\\% Processor Time", 0, &hCounter);
		PdhGetRawCounterValue(hCounter, nullptr, &rawCounter);
	}
	CPUPerformanceCounter::~CPUPerformanceCounter() { PdhCloseQuery(hQuery); }
	float CPUPerformanceCounter::TakeValue() {
		PDH_RAW_COUNTER	rawValue;
		PDH_FMT_COUNTERVALUE fmtValue;
		// 計測
		PdhCollectQueryData(hQuery);
		// 現在のカウンタの値を取得
		PdhGetRawCounterValue(hCounter, nullptr, &rawValue);
		PdhCalculateCounterFromRawValue(hCounter, PDH_FMT_DOUBLE, &rawValue, &rawCounter, &fmtValue);
		rawCounter = rawValue;
		return f_cast(fmtValue.doubleValue);
	}
	MemoryPerformanceCounter::MemoryPerformanceCounter() :pmc{} {
		processID = GetCurrentProcessId();
		process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
		if (!process)STRICT_THROW("メモリへのクエリ失敗");
	}
	MemoryPerformanceCounter::~MemoryPerformanceCounter() = default;
	void MemoryPerformanceCounter::Update() {
		if (!GetProcessMemoryInfo(process, &pmc, sizeof(pmc))) STRICT_THROW("メモリの状況取得に失敗");
	}
	DWORD MemoryPerformanceCounter::GetPaugeFaultCount() { return pmc.PageFaultCount; }
	size_t MemoryPerformanceCounter::GetPeakWorkingSetSize() { return pmc.PeakWorkingSetSize; }
	size_t MemoryPerformanceCounter::GetWorkingSetSize() { return pmc.WorkingSetSize; }
	size_t MemoryPerformanceCounter::GetQuotaPeakPagedPoolUsage() { return pmc.QuotaPeakPagedPoolUsage; }
	size_t MemoryPerformanceCounter::GetQuotaPagedPoolUsage() { return pmc.QuotaPagedPoolUsage; }
	size_t MemoryPerformanceCounter::GetQuotaPeakNonPagedPoolUsage() { return pmc.QuotaPeakNonPagedPoolUsage; }
	size_t MemoryPerformanceCounter::GetQuotaNonPagedPoolUsage() { return pmc.QuotaNonPagedPoolUsage; }
	size_t MemoryPerformanceCounter::GetPagefileUsage() { return pmc.PagefileUsage; }
	size_t MemoryPerformanceCounter::GetPeakPagefileUsage() { return pmc.PeakPagefileUsage; }
}