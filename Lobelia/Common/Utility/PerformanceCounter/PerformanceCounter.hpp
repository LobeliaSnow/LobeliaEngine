#pragma once
#include <Psapi.h> 
#include "pdh.h"
#pragma comment(lib,"pdh.lib")

namespace Lobelia::Utility {
	//���̃T�C�g�̕�������
	//http://atelier-peppe.jp/programTips/MFC/MFC_26.html
	//���̃J�E���^�[�ŋA���Ă���͎̂��v���Z�X��CPU�g�p�����ǂꂾ���H���Ă��邩���擾�ł���B
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
	//�R�����g�̓��e�͂�������Q��
	//https://blog.goo.ne.jp/masaki_goo_2006/e/7a33fc63935919da2fd3f3bec37f8695
	class MemoryPerformanceCounter {
	public:
		MemoryPerformanceCounter();
		~MemoryPerformanceCounter();
		void Update();
		//�y�[�W �t�H�[���g��
		DWORD GetPaugeFaultCount();
		//�ő像�[�L���O�Z�b�g
		size_t GetPeakWorkingSetSize();
		//���݃��[�L���O�Z�b�g
		size_t GetWorkingSetSize();
		//�ő�y�[�W�v�[���̎g�p�T�C�Y
		size_t GetQuotaPeakPagedPoolUsage();
		//���݃y�[�W�v�[���̎g�p�T�C�Y
		size_t GetQuotaPagedPoolUsage();
		//�ő��y�[�W�v�[���̎g�p�T�C�Y
		size_t GetQuotaPeakNonPagedPoolUsage();
		//���ݔ�y�[�W�v�[���̎g�p�T�C�Y
		size_t GetQuotaNonPagedPoolUsage();
		//���݃y�[�W���O�t�@�C���̎g�p�T�C�Y
		size_t GetPagefileUsage();
		//�ő�y�[�W���O�t�@�C���̎g�p�T�C�Y
		size_t GetPeakPagefileUsage();
	private:
		DWORD processID;
		HANDLE process;
		PROCESS_MEMORY_COUNTERS pmc;
	};
}