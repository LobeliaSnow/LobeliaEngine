#pragma once
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace Lobelia::Utility {
	//mutexとthreadを合わせたクラス
	//一旦future&atomicの戻り値取得は忘れる
	//リソースのロックとして成り立つのはあくまでも同一インスタンスを使いまわす場合のみ
	//これを使った管理や、このクラスの拡張を少し考える
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
			//スレッドの追加
			ID id = thread.get_id();
			threads[id] = std::move(thread);
			return id;
		}
		void EndThread(ID id) {
			threads[id].join();
			//デストラクタによるアンロック
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