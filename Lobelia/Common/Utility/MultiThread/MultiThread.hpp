#pragma once
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace Lobelia::Utility {
	//mutex��thread�����킹���N���X
	//��Ufuture&atomic�̖߂�l�擾�͖Y���
	//���\�[�X�̃��b�N�Ƃ��Đ��藧�̂͂����܂ł�����C���X�^���X���g���܂킷�ꍇ�̂�
	//������g�����Ǘ���A���̃N���X�̊g���������l����
	class MultiThread {
	public:
		using LockObject = std::unique_lock<std::shared_mutex>;
		using ID = std::thread::id;
	public:
		MultiThread() = default;
		virtual ~MultiThread() = default;
		template<class Function, class... Args> ID BeginThread(Function func, Args... args) {
			std::thread thread(
				[=] {
				LockObject lock = GetLock();
				func(args...);
			}
			);
			//�X���b�h�̒ǉ�
			ID id = thread.get_id();
			threads[id] = std::move(thread);
			return id;
		}
		void EndThread(ID id) {
			threads[id].join();
			//�f�X�g���N�^�ɂ��A�����b�N
			threads.erase(id);
		}
		void Lock(LockObject& lock) { lock = GetLock(); }
		void UnLock(LockObject& lock) { lock.unlock(); }
		int Size() { return i_cast(threads.size()); }
		bool IsThread(ID id) { return threads.find(id) != threads.end(); }
	private:
		LockObject GetLock() { return LockObject(mutex); }
	private:
		std::shared_mutex mutex;
		std::map<ID, std::thread> threads;
	};
}