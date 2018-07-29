#pragma once
#include <memory>
#include <vector>
#include <algorithm>

#ifndef Interface
#define Interface abstract
#endif
//---------------------------------------------------------------------------------------------
//	ここでは汎用化されたデザインパターンが定義されている
//---------------------------------------------------------------------------------------------
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//	Observerパターン
	//---------------------------------------------------------------------------------------------
	//通知受け取る 意味 観察者
	template<class Type, class Key = std::string> class Observer Interface {
		template<class Type, class Key> friend class Observable;
	public:
		Observer() = default;
		virtual ~Observer() = default;
	private:
		//更新
		virtual void Update(Type* subject, const Key& key) = 0;
		//通知可能な値か否か？
		virtual bool ShouldNotify(const Key& key) { return true; };
	};
	//通知を送る 意味 観測可能な 
	template<class Type, class Key = std::string> class Observable {
	private:
		//listでよくね
		std::vector<Observer<Type, Key>*> observers;
	public:
		Observable() = default;
		virtual ~Observable() { observers.clear(); }
		//オブザーバーの追加
		void AddObserver(Observer<Type, Key>* observer) {
			observers.push_back(observer);
		}
		//指定したオブザーバーの全件削除
		void DeleteObserver(Observer<Type, Key>* observer) {
			//erase-remove idiom
			//std::removeで一致する値を切り詰める。
			//その際に切り詰められた後の末尾のイテレーター返ってくるので、そこから先を一斉削除
			observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
		}
		//通知
		void Notify(const Key& key) {
			for each(auto& observer in observers) {
				//通知できる値か確かめて更新する
				if (observer->ShouldNotify(key))observer->Update(static_cast<Type*>(this), key);
			}
		}
		void Notify(Key&& key) { Notify(key); }
	};
	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

}