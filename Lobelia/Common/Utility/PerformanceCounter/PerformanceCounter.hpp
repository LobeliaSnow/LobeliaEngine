#pragma once
#include <Psapi.h> 
#include "pdh.h"
#pragma comment(lib,"pdh.lib")

namespace Lobelia::Utility {
	//このサイトの物を実装
	//http://atelier-peppe.jp/programTips/MFC/MFC_26.html
	//このカウンターで帰ってくるのは自プロセスがCPU使用率をどれだけ食っているかを取得できる。
	class CPUPerformanceCounter {
	public:
		CPUPerformanceCounter();
		~CPUPerformanceCounter();
		float TakeValue();
	private:
		HQUERY hQuery;
		HCOUNTER hCounter;
		PDH_RAW_COUNTER rawCounter;
	};
	//コメントの内容はここから参照
	//https://blog.goo.ne.jp/masaki_goo_2006/e/7a33fc63935919da2fd3f3bec37f8695
	class MemoryPerformanceCounter {
	public:
		MemoryPerformanceCounter();
		~MemoryPerformanceCounter();
		void Update();
		//ページ フォールト数
		DWORD GetPaugeFaultCount();
		//最大ワーキングセット
		size_t GetPeakWorkingSetSize();
		//現在ワーキングセット
		size_t GetWorkingSetSize();
		//最大ページプールの使用サイズ
		size_t GetQuotaPeakPagedPoolUsage();
		//現在ページプールの使用サイズ
		size_t GetQuotaPagedPoolUsage();
		//最大非ページプールの使用サイズ
		size_t GetQuotaPeakNonPagedPoolUsage();
		//現在非ページプールの使用サイズ
		size_t GetQuotaNonPagedPoolUsage();
		//現在ページングファイルの使用サイズ
		size_t GetPagefileUsage();
		//最大ページングファイルの使用サイズ
		size_t GetPeakPagefileUsage();
	private:
		DWORD processID;
		HANDLE process;
		PROCESS_MEMORY_COUNTERS pmc;
	};
}