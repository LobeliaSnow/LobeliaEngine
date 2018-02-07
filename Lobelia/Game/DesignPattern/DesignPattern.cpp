#include "DesignPattern.hpp"
#include <Windows.h>

//ここでは使用例を公開
namespace Lobelia::Game {
	//observerパターン例
	class Sender :public Observable<Sender, int> {
	public:
	};
	class Reciver :public Observer<Sender, int> {
	public:
		void Update(Sender* subject, const int& key) override {
			if (key == 0)OutputDebugString("ぴこーん0(通知が来た音)\n");
			else if (key == 1)OutputDebugString("ぴこーん1(通知が来た音)\n");
			else OutputDebugString("ががー(バグ)\n");
		}
	};
	namespace {
		void UseExample() {
			//observerパターン紹介
			Sender sender;
			Reciver reciver;
			sender.AddObserver(&reciver);
			sender.Notify(0);
			sender.Notify(1);
			sender.Notify(2);
			sender.DeleteObserver(&reciver);
			//--end--

		}
	}
}